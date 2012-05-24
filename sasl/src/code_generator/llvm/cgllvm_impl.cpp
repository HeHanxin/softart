#include <sasl/include/code_generator/llvm/cgllvm_impl.imp.h>

#include <sasl/include/code_generator/llvm/utility.h>
#include <sasl/include/code_generator/llvm/cgllvm_caster.h>
#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/semantic_infos.imp.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/caster.h>
#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/syntax_tree/program.h>

#include <sasl/enums/builtin_types.h>
#include <sasl/enums/enums_utility.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Target/TargetData.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Constants.h>
#include <llvm/Support/TargetSelect.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/foreach.hpp>
#include <boost/utility.hpp>
#include <boost/format.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

using namespace sasl::syntax_tree;
using namespace sasl::semantic;
using namespace llvm;
using namespace sasl::utility;

using boost::bind;
using boost::any_cast;
using boost::addressof;
using boost::format;

using std::vector;

#define SASL_VISITOR_TYPE_NAME cgllvm_impl

BEGIN_NS_SASL_CODE_GENERATOR();

llvm::DefaultIRBuilder* cgllvm_impl::builder() const
{
	return mod->builder().get();
}

llvm::LLVMContext& cgllvm_impl::context() const
{
	return mod->context();
}

llvm::Module* cgllvm_impl::module() const
{
	return mod->module();
}

cgllvm_sctxt* cgllvm_impl::node_ctxt( node* n, bool create_if_need /*= false */ )
{
	shared_ptr<cgllvm_sctxt>& ret = ctxts[n];
	if( ret ){
		return ret.get();
	} else if( create_if_need ){
		ret = create_codegen_context<cgllvm_sctxt>( n->as_handle() );
		return ret.get();
	}

	return NULL;
}

shared_ptr<llvm_module> cgllvm_impl::cg_module() const
{
	return mod;
}

bool cgllvm_impl::generate( module_si* mod, abi_info const* abii )
{
	msi = mod;
	this->abii = abii;

	if( msi ){
		assert( msi->root() );
		assert( msi->root()->node() );
		msi->root()->node()->accept( this, NULL );
		return true;
	}

	return false;
}

cgllvm_impl::~cgllvm_impl()
{
	// if( target_data ){ delete target_data; }
}

shared_ptr<symbol> cgllvm_impl::find_symbol( cgllvm_sctxt* data, std::string const& str )
{
	return data->env().sym.lock()->find( str );
}

function_t* cgllvm_impl::get_function( std::string const& name ) const
{
	shared_ptr<symbol> callee_sym = msi->root()->find_overloads(name)[0];
	return &( const_cast<cgllvm_impl*>(this)->node_ctxt( callee_sym->node() )->data().self_fn );
}

SASL_VISIT_DEF( variable_expression ){
	shared_ptr<symbol> declsym = sc_env_ptr(data)->sym.lock()->find( v.var_name->str );
	assert( declsym && declsym->node() );

	sc_ptr(data)->value() = node_ctxt( declsym->node(), false )->value();
	sc_ptr(data)->get_tysp() = node_ctxt( declsym->node(), false )->get_tysp();
	sc_ptr(data)->data().semantic_mode = node_ctxt( declsym->node(), false )->data().semantic_mode;

	// sc_data_ptr(data)->hint_name = v.var_name->str.c_str();
	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( binary_expression ){
	any child_ctxt_init = *data;
	sc_ptr(child_ctxt_init)->clear_data();
	any child_ctxt;

	if( v.op == operators::logic_and || v.op == operators::logic_or ){
		bin_logic( v, data );
	} else {
		visit_child( child_ctxt, child_ctxt_init, v.left_expr );
		visit_child( child_ctxt, child_ctxt_init, v.right_expr );

		if(/**/v.op == operators::assign
			|| v.op == operators::add_assign
			|| v.op == operators::sub_assign
			|| v.op == operators::mul_assign
			|| v.op == operators::div_assign
			|| v.op == operators::mod_assign
			|| v.op == operators::lshift_assign
			|| v.op == operators::rshift_assign
			|| v.op == operators::bit_and_assign
			|| v.op == operators::bit_or_assign
			|| v.op == operators::bit_xor_assign
			)
		{
			bin_assign( v, data );
		}
		else
		{
			std::string op_name = operator_name(v.op);

			shared_ptr<type_info_si> larg_tsi = extract_semantic_info<type_info_si>(v.left_expr);
			shared_ptr<type_info_si> rarg_tsi = extract_semantic_info<type_info_si>(v.right_expr);

			std::vector< shared_ptr<expression> > args;
			args.push_back( v.left_expr );
			args.push_back( v.right_expr );

			symbol::overloads_t overloads = sc_env_ptr(data)->sym.lock()->find_overloads( op_name, caster, args, NULL );
			EFLIB_ASSERT( overloads.size() == 1, "No or more an one overloads." );

			boost::shared_ptr<function_type> op_proto = overloads[0]->node()->as_handle<function_type>();

			type_info_si* p0_tsi = op_proto->params[0]->si_ptr<type_info_si>();
			type_info_si* p1_tsi = op_proto->params[1]->si_ptr<type_info_si>();

			// cast value type to match proto type.
			if( p0_tsi->entry_id() != larg_tsi->entry_id() ){
				if( ! node_ctxt( p0_tsi->type_info() ) ){
					visit_child( child_ctxt, child_ctxt_init, op_proto->params[0]->param_type );
				}
				caster->cast( p0_tsi->type_info(), v.left_expr );
			}
			if( p1_tsi->entry_id() != rarg_tsi->entry_id() ){
				if( ! node_ctxt( p1_tsi->type_info() ) ){
					visit_child( child_ctxt, child_ctxt_init, op_proto->params[1]->param_type );
				}
				caster->cast( p1_tsi->type_info(), v.right_expr );
			}

			// use type-converted value to generate code.
			value_t lval = node_ctxt(v.left_expr)->get_rvalue();
			value_t rval = node_ctxt(v.right_expr)->get_rvalue();

			value_t retval;

			builtin_types lbtc = p0_tsi->type_info()->tycode;
			builtin_types rbtc = p1_tsi->type_info()->tycode;

			if( v.op == operators::add ){
				retval = service()->emit_add(lval, rval);
			} else if ( v.op == operators::mul ) {
				retval = service()->emit_mul_comp(lval, rval);
			} else if ( v.op == operators::sub ) {
				retval = service()->emit_sub(lval, rval);
			} else if( v.op == operators::div ){
				retval = service()->emit_div(lval, rval);
			} else if( v.op == operators::mod ){
				retval = service()->emit_mod(lval, rval);
			} else if( v.op == operators::left_shift ) {
				retval = service()->emit_lshift( lval, rval );
			} else if( v.op == operators::right_shift ) {
				retval = service()->emit_rshift( lval, rval );
			} else if( v.op == operators::bit_and ) {
				retval = service()->emit_bit_and( lval, rval );
			} else if( v.op == operators::bit_or ) {
				retval = service()->emit_bit_or( lval, rval );
			} else if( v.op == operators::bit_xor ) {
				retval = service()->emit_bit_xor( lval, rval );
			} else if( v.op == operators::less ) {
				retval = service()->emit_cmp_lt( lval, rval );
			} else if( v.op == operators::less_equal ){
				retval = service()->emit_cmp_le( lval, rval );
			} else if( v.op == operators::equal ){
				retval = service()->emit_cmp_eq( lval, rval );
			} else if( v.op == operators::greater_equal ){
				retval = service()->emit_cmp_ge( lval, rval );
			} else if( v.op == operators::greater ){
				retval = service()->emit_cmp_gt( lval, rval );	
			} else if( v.op == operators::not_equal ){
				retval = service()->emit_cmp_ne( lval, rval );
			} else {
				EFLIB_ASSERT_UNIMPLEMENTED0( ( operators::to_name(v.op) + " not be implemented." ).c_str() );
			}

			assert(retval.hint() == op_proto->si_ptr<type_info_si>()->type_info()->tycode);

			sc_ptr(data)->value() = retval;
			node_ctxt(v, true)->copy( sc_ptr(data) );
		}
	}
}

SASL_VISIT_DEF( constant_expression ){

	any child_ctxt_init = *data;
	any child_ctxt;

	boost::shared_ptr<const_value_si> c_si = extract_semantic_info<const_value_si>(v);
	if( ! node_ctxt( c_si->type_info() ) ){
		visit_child( child_ctxt, child_ctxt_init, c_si->type_info() );
	}

	cgllvm_sctxt* const_ctxt = node_ctxt( c_si->type_info() );

	value_tyinfo* tyinfo = const_ctxt->get_typtr();
	assert( tyinfo );

	value_t val;
	if( c_si->value_type() == builtin_types::_sint32 ){
		val = service()->create_constant_scalar( c_si->value<int32_t>(), tyinfo, tyinfo->hint() );
	} else if ( c_si->value_type() == builtin_types::_uint32 ) {
		val = service()->create_constant_scalar( c_si->value<uint32_t>(), tyinfo, tyinfo->hint() );
	} else if ( c_si->value_type() == builtin_types::_float ) {
		val = service()->create_constant_scalar( c_si->value<double>(), tyinfo, tyinfo->hint() );
	} else {
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	sc_ptr(data)->value() = val;

	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( call_expression ){
	any child_ctxt_init = *data;
	sc_ptr(&child_ctxt_init)->clear_data();

	any child_ctxt;

	call_si* csi = v.si_ptr<call_si>();
	if( csi->is_function_pointer() ){
		visit_child( child_ctxt, child_ctxt_init, v.expr );
		EFLIB_ASSERT_UNIMPLEMENTED();
	} else {
		// Get LLVM Function
		symbol* fn_sym = csi->overloaded_function();
		shared_ptr<function_type> proto = fn_sym->node()->as_handle<function_type>();
		
		vector<value_t> args;
		for( size_t i_arg = 0; i_arg < v.args.size(); ++i_arg )
		{
			visit_child( child_ctxt, child_ctxt_init, v.args[i_arg] );

			type_info_si* arg_tisi = v.args[i_arg]->si_ptr<type_info_si>();
			type_info_si* par_tisi = proto->params[i_arg]->si_ptr<type_info_si>();
			
			if( par_tisi->entry_id() != arg_tisi->entry_id() )
			{
				if( !node_ctxt( par_tisi->type_info() ) )
				{
					visit_child( child_ctxt, child_ctxt_init, proto->params[i_arg]->param_type );
				}
				caster->cast( par_tisi->type_info(), v.args[i_arg] );
			}

			cgllvm_sctxt* arg_ctxt = node_ctxt( v.args[i_arg], false );
			args.push_back( arg_ctxt->value() );
		}
		
		function_t fn = service()->fetch_function( proto );
		value_t rslt  = service()->emit_call( fn, args );

		cgllvm_sctxt* expr_ctxt = node_ctxt( v, true );
		expr_ctxt->data().val = rslt;
		expr_ctxt->data().tyinfo = fn.get_return_ty();

		sc_ptr(data)->copy( expr_ctxt );
	}
}

SASL_VISIT_DEF( index_expression )
{
	any child_ctxt_init = *data;
	sc_ptr(&child_ctxt_init)->clear_data();

	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.expr );
	visit_child( child_ctxt, child_ctxt_init, v.index_expr );
	cgllvm_sctxt* expr_ctxt  = node_ctxt(v.expr);
	cgllvm_sctxt* index_ctxt = node_ctxt(v.index_expr);
	assert( expr_ctxt && index_ctxt );

	cgllvm_sctxt* ret_ctxt = node_ctxt( v, true );
	ret_ctxt->value()		= service()->emit_extract_elem( expr_ctxt->value(), index_ctxt->value() );
	ret_ctxt->data().tyinfo = service()->create_tyinfo( v.si_ptr<type_info_si>()->type_info() );
	
	sc_ptr(data)->data(ret_ctxt);
}

SASL_VISIT_DEF( builtin_type ){

	shared_ptr<type_info_si> tisi = extract_semantic_info<type_info_si>( v );

	cgllvm_sctxt* pctxt = node_ctxt( tisi->type_info(), true );

	if( !pctxt->get_typtr() ){
		shared_ptr<value_tyinfo> bt_tyinfo = service()->create_tyinfo( v.as_handle<tynode>() );
		assert( bt_tyinfo );
		pctxt->data().tyinfo = bt_tyinfo;

		std::string tips = v.tycode.name() + std::string(" was not supported yet.");
		EFLIB_ASSERT_AND_IF( pctxt->data().tyinfo, tips.c_str() ){
			return;
		}
	}

	sc_ptr( data )->data( pctxt );
	return;
}

SASL_VISIT_DEF( parameter ){
	sc_ptr(data)->clear_data();

	any child_ctxt_init = *data;
	sc_ptr(child_ctxt_init)->clear_data();

	any child_ctxt;
	visit_child( child_ctxt, child_ctxt_init, v.param_type );

	if( v.init ){
		visit_child( child_ctxt, child_ctxt_init, v.init );
	}

	sc_data_ptr(data)->val = sc_data_ptr(&child_ctxt)->val;
	sc_data_ptr(data)->tyinfo = sc_data_ptr(&child_ctxt)->tyinfo;
	node_ctxt(v, true)->copy( sc_ptr(data) );
}

// Generate normal function code.
SASL_VISIT_DEF( function_type ){
	sc_env_ptr(data)->sym = v.symbol();

	cgllvm_sctxt* fnctxt = node_ctxt(v.symbol()->node(), true);
	if( !fnctxt->data().self_fn ){
		create_fnsig( v, data );
		node_ctxt(v.symbol()->node(), true)->data().self_fn = sc_data_ptr(data)->self_fn;
	}

	if ( v.body ){
		FUNCTION_SCOPE( sc_data_ptr(data)->self_fn );

		service()->function_beg();
		service()->fn().allocation_block( service()->new_block(".alloc", true) );
		create_fnargs( v, data );
		create_fnbody( v, data );
		service()->function_end();
	}

	// Here use the definition node.
	node_ctxt(v.symbol()->node(), true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( struct_type ){
	// Create context.
	// Declarator visiting need parent information.
	cgllvm_sctxt* ctxt = node_ctxt(v, true);

	// A struct is visited at definition type.
	// If the visited again, it must be as an alias_type.
	// So return environment directly.
	if( ctxt->data().tyinfo ){
		sc_ptr(data)->data(ctxt);
		return;
	}

	std::string name = v.symbol()->mangled_name();

	// Init data.
	any child_ctxt_init = *data;
	sc_ptr(child_ctxt_init)->clear_data();
	sc_env_ptr(&child_ctxt_init)->parent_struct = ctxt;

	any child_ctxt;
	BOOST_FOREACH( shared_ptr<declaration> const& decl, v.decls ){
		visit_child( child_ctxt, child_ctxt_init, decl );
	}
	sc_data_ptr(data)->tyinfo = service()->create_tyinfo( v.si_ptr<type_info_si>()->type_info() );

	ctxt->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( array_type )
{
	cgllvm_sctxt* ctxt = node_ctxt(v, true);
	if( ctxt->data().tyinfo ){
		sc_ptr(data)->data(ctxt);
		return;
	}

	any child_ctxt_init = *data;
	any child_ctxt;
	sc_ptr(child_ctxt_init)->clear_data();

	visit_child(child_ctxt, child_ctxt_init, v.elem_type);
	sc_data_ptr(data)->tyinfo = service()->create_tyinfo(v.si_ptr<type_info_si>()->type_info());

	ctxt->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( variable_declaration ){
	// Visit type info
	any child_ctxt_init = *data;
	sc_ptr(child_ctxt_init)->clear_data();
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.type_info );

	sc_env_ptr(&child_ctxt_init)->tyinfo = sc_data_ptr(&child_ctxt)->tyinfo;

	BOOST_FOREACH( shared_ptr<declarator> const& dclr, v.declarators ){
		visit_child( child_ctxt, child_ctxt_init, dclr );
	}

	sc_data_ptr(data)->declarator_count = static_cast<int>( v.declarators.size() );

	sc_data_ptr(data)->tyinfo = sc_data_ptr(&child_ctxt)->tyinfo;
	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( declarator ){
	// local or member.
	// TODO TBD: Support member function and nested structure ?
	if( service()->in_function() ){
		visit_local_declarator( v, data );
	} else if( sc_env_ptr(data)->parent_struct ){
		visit_member_declarator( v, data );
	} else {
		visit_global_declarator(v, data);
	}
}

SASL_VISIT_DEF( expression_initializer ){
	any child_ctxt_init = *data;
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.init_expr );

	shared_ptr<type_info_si> init_tsi = extract_semantic_info<type_info_si>(v.as_handle());
	shared_ptr<type_info_si> var_tsi = extract_semantic_info<type_info_si>(sc_env_ptr(data)->variable_to_fill.lock());

	if( init_tsi->entry_id() != var_tsi->entry_id() ){
		caster->cast( var_tsi->type_info(), v.init_expr );
	}

	sc_ptr(data)->copy( node_ctxt(v.init_expr, false) );
	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( expression_statement ){
	any child_ctxt_init = *data;
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.expr );

	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( declaration_statement ){
	any child_ctxt_init = *data;
	any child_ctxt;

	BOOST_FOREACH( shared_ptr<declaration> const& decl, v.decls )
	{
		visit_child( child_ctxt, child_ctxt_init, decl );
	}
	
	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( jump_statement )
{
	any child_ctxt_init = *data;
	any child_ctxt;

	if (v.jump_expr){
		visit_child( child_ctxt, child_ctxt_init, v.jump_expr );
	}

	if ( v.code == jump_mode::_return ){
		visit_return(v, data);
	} else if ( v.code == jump_mode::_continue ){
		visit_continue(v, data);
	} else if ( v.code == jump_mode::_break ){
		visit_break( v, data );
	}

	// Restart a new block for sealing the old block.
	service()->new_block(".restart", true);
	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( program )
{	
	// Create module.
	assert( !mod );
	mod = create_codegen_context<llvm_module_impl>( v.as_handle() );
	if( !mod ) return;

	// Initialization.
	mod->create_module( v.name );
	service()->initialize( mod.get(),
		boost::bind(static_cast<cgllvm_sctxt*(cgllvm_impl::*)(node*, bool)>(&cgllvm_impl::node_ctxt), this, _1, _2)
		);

	typedef cgllvm_sctxt* (fn_proto_t)( boost::shared_ptr<sasl::syntax_tree::node> const& );
	typedef cgllvm_sctxt* (cgllvm_impl::*mem_fn_proto_t) ( boost::shared_ptr<sasl::syntax_tree::node> const&, bool );
	boost::function<fn_proto_t> ctxt_getter
		= boost::bind( static_cast<mem_fn_proto_t>(&cgllvm_impl::node_ctxt), this, _1, false );
	caster = create_caster( ctxt_getter, service() );
	add_builtin_casts( caster, msi->pety() );
	
	process_intrinsics( v, data );

	// Some other initializations.
	before_decls_visit( v, data );

	// visit declarations
	any child_ctxt = cgllvm_sctxt();
	for( vector< shared_ptr<declaration> >::iterator
		it = v.decls.begin(); it != v.decls.end(); ++it )
	{
		visit_child( child_ctxt, (*it) );
	}
}

SASL_SPECIFIC_VISIT_DEF( before_decls_visit, program )
{
	EFLIB_UNREF_PARAM(data);
	EFLIB_UNREF_PARAM(v);
	
	llvm::InitializeNativeTarget();

	TargetMachine* tm = EngineBuilder(module()).selectTarget();
	target_data = tm->getTargetData();
}

SASL_SPECIFIC_VISIT_DEF( visit_member_declarator, declarator ){
	
	shared_ptr<value_tyinfo> decl_ty = sc_env_ptr(data)->tyinfo;
	assert(decl_ty);

	// Needn't process init expression now.
	storage_si* si = v.si_ptr<storage_si>();
	sc_data_ptr(data)->tyinfo = decl_ty;
	sc_data_ptr(data)->val = service()->create_value(decl_ty.get(), NULL, vkind_swizzle, abi_unknown );
	sc_data_ptr(data)->val.index( si->mem_index() );

	node_ctxt(v, true)->copy( sc_ptr(data) );
}
SASL_SPECIFIC_VISIT_DEF( visit_global_declarator, declarator ){
	sc_env_ptr(data)->sym = v.symbol();
	node_ctxt(v, true)->copy( sc_ptr(data) );
}
SASL_SPECIFIC_VISIT_DEF( visit_local_declarator , declarator ){
	any child_ctxt_init = *data;
	sc_ptr(child_ctxt_init)->clear_data();

	any child_ctxt;

	sc_data_ptr(data)->tyinfo = sc_env_ptr(data)->tyinfo;
	sc_data_ptr(data)->val = service()->create_variable( sc_data_ptr(data)->tyinfo.get(), local_abi( v.si_ptr<storage_si>()->c_compatible() ), v.name->str );

	if ( v.init ){
		sc_env_ptr(&child_ctxt_init)->variable_to_fill = v.as_handle();
		visit_child( child_ctxt, child_ctxt_init, v.init );
		sc_data_ptr(data)->val.store( sc_ptr(&child_ctxt)->value() );
	}

	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_SPECIFIC_VISIT_DEF( create_fnsig, function_type ){
	any child_ctxt_init = *data;
	sc_ptr(child_ctxt_init)->clear_data();

	any child_ctxt;

	// Generate return type node.
	visit_child( child_ctxt, child_ctxt_init, v.retval_type );
	shared_ptr<value_tyinfo> ret_ty = sc_data_ptr(&child_ctxt)->tyinfo;
	assert( ret_ty );

	// Generate parameters.
	BOOST_FOREACH( shared_ptr<parameter> const& par, v.params ){
		visit_child( child_ctxt, child_ctxt_init, par );
	}

	sc_data_ptr(data)->self_fn = service()->fetch_function( v.as_handle<function_type>() );
}
SASL_SPECIFIC_VISIT_DEF( create_fnargs, function_type ){

	EFLIB_UNREF_PARAM(data);

	// Register arguments names.
	assert( service()->fn().arg_size() == v.params.size() );

	service()->fn().return_name( ".ret" );
	size_t i_arg = 0;
	BOOST_FOREACH( shared_ptr<parameter> const& par, v.params )
	{
		sctxt_handle par_ctxt = node_ctxt( par );
		service()->fn().arg_name( i_arg, par->symbol()->unmangled_name() );
		par_ctxt->value() = service()->fn().arg( i_arg++ );
	}
}
SASL_SPECIFIC_VISIT_DEF( create_fnbody, function_type ){
	any child_ctxt_init = *data;
	any child_ctxt;

	service()->new_block(".body", true);
	visit_child( child_ctxt, child_ctxt_init, v.body );

	service()->clean_empty_blocks();
}

SASL_SPECIFIC_VISIT_DEF( visit_return, jump_statement ){
	(data); (v);
	if ( !v.jump_expr ){
		service()->emit_return();
	} else {
		shared_ptr<tynode> fn_retty = service()->fn().fnty->retval_type;
		tid_t fret_tid = fn_retty->si_ptr<type_info_si>()->entry_id();
		tid_t expr_tid = v.jump_expr->si_ptr<type_info_si>()->entry_id();
		if( fret_tid != expr_tid )
		{
			caster->cast( fn_retty, v.jump_expr );
		}
		service()->emit_return( node_ctxt(v.jump_expr)->value(), service()->param_abi( service()->fn().c_compatible ) );
	}
}

/* Make binary assignment code.
*    Note: Right argument is assignee, and left argument is value.
*/
SASL_SPECIFIC_VISIT_DEF( bin_assign, binary_expression ){
	any child_ctxt_init = *data;
	sc_ptr(child_ctxt_init)->clear_data();
	any child_ctxt;

	std::string op_name = operator_name(v.op);

	type_info_si* larg_tsi =  v.left_expr->si_ptr<type_info_si>();

	std::vector< shared_ptr<expression> > args;
	args.push_back( v.left_expr );
	args.push_back( v.right_expr );

	symbol::overloads_t overloads = sc_env_ptr(data)->sym.lock()->find_overloads( op_name, caster, args, NULL );
	EFLIB_ASSERT( overloads.size() == 1, "No or more an one overloads." );

	shared_ptr<function_type> op_proto = overloads[0]->node()->as_handle<function_type>();

	type_info_si* p0_tsi = op_proto->params[0]->si_ptr<type_info_si>();
	if( p0_tsi->entry_id() != larg_tsi->entry_id() )
	{
		if( !node_ctxt( p0_tsi->type_info() ) )
		{
			visit_child( child_ctxt, child_ctxt_init, op_proto->params[0]->param_type );
		}
		caster->cast( p0_tsi->type_info(), v.left_expr );
	}

	// Evaluated by visit(binary_expression)
	cgllvm_sctxt* lctxt = node_ctxt( v.left_expr );
	cgllvm_sctxt* rctxt = node_ctxt( v.right_expr );

	value_t val;
	/**/ if( v.op == operators::add_assign )
	{
		val = service()->emit_add( rctxt->value(), lctxt->value() );
	}
	else if( v.op == operators::sub_assign )
	{
		val = service()->emit_sub( rctxt->value(), lctxt->value() );
	}
	else if( v.op == operators::mul_assign )
	{
		val = service()->emit_mul_comp( rctxt->value(), lctxt->value() );
	}
	else if( v.op == operators::div_assign )
	{
		val = service()->emit_div( rctxt->value(), lctxt->value() );
	}
	else if( v.op == operators::mod_assign )
	{
		val = service()->emit_mod( rctxt->value(), lctxt->value() );
	}
	else if( v.op == operators::lshift_assign )
	{
		val = service()->emit_lshift( rctxt->value(), lctxt->value() );
	}
	else if( v.op == operators::rshift_assign )
	{
		val = service()->emit_rshift( rctxt->value(), lctxt->value() );
	}
	else if( v.op == operators::bit_and_assign )
	{
		val = service()->emit_bit_and( rctxt->value(), lctxt->value() );
	}
	else if( v.op == operators::bit_or_assign )
	{
		val = service()->emit_bit_or( rctxt->value(), lctxt->value() );
	}
	else if( v.op == operators::bit_xor_assign )
	{
		val = service()->emit_bit_xor( rctxt->value(), lctxt->value() );
	}
	else if( v.op == operators::sub_assign )
	{
		val = service()->emit_add( rctxt->value(), lctxt->value() );
	}
	else
	{
		assert( v.op == operators::assign );
		val = lctxt->value();
	}

	rctxt->value().store(val);

	cgllvm_sctxt* pctxt = node_ctxt(v, true);
	pctxt->data( rctxt->data() );
	pctxt->env( sc_ptr(data) );
}

SASL_SPECIFIC_VISIT_DEF( process_intrinsics, program )
{
	EFLIB_UNREF_PARAM(data);
	EFLIB_UNREF_PARAM(v);

	service()->register_external_intrinsic();

	vector< shared_ptr<symbol> > const& intrinsics = msi->intrinsics();

	BOOST_FOREACH( shared_ptr<symbol> const& intr, intrinsics ){
		shared_ptr<function_type> intr_fn = intr->node()->as_handle<function_type>();
		storage_si* intrin_ssi = intr_fn->si_ptr<storage_si>();
		bool external = intrin_ssi->external_compatible();

		// If intrinsic is not invoked, we don't generate code for it.
		if( !intrin_ssi->is_invoked() && !external ){ continue;	}

		any child_ctxt = cgllvm_sctxt();

		visit_child( child_ctxt, intr_fn );
		// Deal with external functions. External function has nobody.
		if ( external ){ continue; }

		cgllvm_sctxt* intrinsic_ctxt = node_ctxt( intr_fn, false );
		assert( intrinsic_ctxt );

		service()->push_fn( intrinsic_ctxt->data().self_fn );
		scope_guard<void> pop_fn_on_exit( bind( &cg_service::pop_fn, service() ) );

		service()->fn().allocation_block( service()->new_block(".alloc", true) );
		insert_point_t ip_body = service()->new_block( ".body", true );

		// Parse Parameter Informations
		vector< shared_ptr<tynode> > par_tys;
		vector<builtin_types> par_tycodes;
		vector<cgllvm_sctxt*> par_ctxts;

		BOOST_FOREACH( shared_ptr<parameter> const& par, intr_fn->params )
		{
			par_tys.push_back( par->si_ptr<type_info_si>()->type_info() );
			assert( par_tys.back() );
			par_tycodes.push_back( par_tys.back()->tycode );
			par_ctxts.push_back( node_ctxt(par, false) );
			assert( par_ctxts.back() );
		}

		shared_ptr<value_tyinfo> result_ty = service()->fn().get_return_ty();
		
		service()->fn().inline_hint();

		// Process Intrinsic
		if( intr->unmangled_name() == "mul" )
		{
			
			assert( par_tys.size() == 2 );

			// Set Argument name
			service()->fn().arg_name( 0, ".lhs" );
			service()->fn().arg_name( 1, ".rhs" );

			value_t ret_val = service()->emit_mul_intrin( service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret_val, service()->param_abi(false) );

		}
		else if( intr->unmangled_name() == "dot" )
		{
			
			assert( par_tys.size() == 2 );

			// Set Argument name
			service()->fn().arg_name( 0, ".lhs" );
			service()->fn().arg_name( 1, ".rhs" );

			value_t ret_val = service()->emit_dot( service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret_val, service()->param_abi(false) );

		}
		else if ( intr->unmangled_name() == "abs" )
		{
			assert( par_tys.size() == 1 );
			service()->fn().arg_name( 0, ".value" );
			value_t ret_val = service()->emit_abs( service()->fn().arg(0) );
			service()->emit_return( ret_val, service()->param_abi(false) );
		}
		else if ( intr->unmangled_name() == "exp"
			|| intr->unmangled_name() == "exp2"
			|| intr->unmangled_name() == "sin"
			|| intr->unmangled_name() == "cos"
			|| intr->unmangled_name() == "tan"
			|| intr->unmangled_name() == "asin"
			|| intr->unmangled_name() == "acos"
			|| intr->unmangled_name() == "atan"
			|| intr->unmangled_name() == "ceil"
			|| intr->unmangled_name() == "floor"
			|| intr->unmangled_name() == "log"
			|| intr->unmangled_name() == "log2"
			|| intr->unmangled_name() == "log10"
			|| intr->unmangled_name() == "rsqrt"
			)
		{
			assert( par_tys.size() == 1 );
			service()->fn().arg_name( 0, ".value" );
			std::string scalar_intrin_name = ( format("sasl.%s.f32") % intr->unmangled_name() ).str();
			value_t ret_val = service()->emit_unary_ps( scalar_intrin_name,service()->fn().arg(0) );
			service()->emit_return( ret_val, service()->param_abi(false) );
		}
		else if(intr->unmangled_name() == "ldexp")
		{
			assert( par_tys.size() == 2 );
			service()->fn().arg_name( 0, ".lhs" );
			service()->fn().arg_name( 1, ".rhs" );
			std::string scalar_intrin_name = ( format("sasl.%s.f32") % intr->unmangled_name() ).str();
			value_t ret_val = service()->emit_bin_ps( scalar_intrin_name, service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret_val, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "sqrt" )
		{
			assert( par_tys.size() == 1 );
			service()->fn().arg_name( 0, ".value" );
			value_t ret_val = service()->emit_sqrt( service()->fn().arg(0) );
			service()->emit_return( ret_val, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "cross" )
		{
			assert( par_tys.size() == 2 );
			service()->fn().arg_name( 0, ".lhs" );
			service()->fn().arg_name( 1, ".rhs" );
			value_t ret_val = service()->emit_cross( service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret_val, service()->param_abi(false) );
		}
		else if ( intr->unmangled_name() == "ddx" || intr->unmangled_name() == "ddy" )
		{
			assert( par_tys.size() == 1 );
			service()->fn().arg_name( 0, ".value" );

			value_t ret_val;
			if( intr->unmangled_name() == "ddx" ){
				ret_val = service()->emit_ddx( service()->fn().arg(0) );
			} else {
				ret_val = service()->emit_ddy( service()->fn().arg(0) );
			}
			service()->emit_return( ret_val, service()->param_abi(false) );
		}
		else if ( intr->unmangled_name() == "tex2D" )
		{
			assert( par_tys.size() == 2 );
			value_t samp = service()->fn().arg(0);
			value_t coord = service()->fn().arg(1);
			value_t ddx = service()->emit_ddx(coord);
			value_t ddy = service()->emit_ddy(coord);
			value_t ret = service()->emit_tex2Dgrad( samp, coord, ddx, ddy );
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if ( intr->unmangled_name() == "tex2Dgrad" )
		{
			assert( par_tys.size() == 4 );
			value_t ret = service()->emit_tex2Dgrad(
				service()->fn().arg(0),
				service()->fn().arg(1),
				service()->fn().arg(2),
				service()->fn().arg(3)
				);
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if ( intr->unmangled_name() == "tex2Dlod" )
		{
			assert( par_tys.size() == 2 );
			value_t ret = service()->emit_tex2Dlod( service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if ( intr->unmangled_name() == "tex2Dbias" )
		{
			assert( par_tys.size() == 2 );
			value_t ret = service()->emit_tex2Dbias( service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "tex2Dproj" )
		{
			assert( par_tys.size() == 2 );
			value_t ret = service()->emit_tex2Dproj( service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if ( intrin_ssi->is_constructor() )
		{
			function_t& fn = service()->fn();
			for(size_t i = 0; i < fn.arg_size(); ++i)
			{
				char name[4] = ".v0";
				name[2] += (char)i;
				fn.arg_name(i, name);
			}
			value_tyinfo* ret_ty = fn.get_return_ty().get();
			builtin_types ret_hint = ret_ty->hint();
			
			if( is_vector(ret_hint) ){
				value_t ret_v = service()->undef_value( ret_hint, service()->param_abi(false) );

				size_t i_scalar = 0;
				for( size_t i_arg = 0; i_arg < fn.arg_size(); ++i_arg ){
					value_t arg_value = fn.arg(i_arg);
					builtin_types arg_hint = arg_value.hint();
					if( is_scalar(arg_hint) ){
						ret_v = service()->emit_insert_val( ret_v, static_cast<int>(i_scalar), arg_value );
						++i_scalar;
					} else if ( is_vector(arg_hint) ) {
						size_t arg_vec_size = vector_size(arg_hint);
						for( size_t i_scalar_in_arg = 0; i_scalar_in_arg < arg_vec_size; ++i_scalar_in_arg ){
							value_t scalar_value = service()->emit_extract_val( arg_value, static_cast<int>(i_scalar_in_arg) );
							ret_v = service()->emit_insert_val( ret_v, static_cast<int>(i_scalar), scalar_value );
							++i_scalar;
						}
					} else {
						// TODO: Error.
						assert( false );
					}
				}
				service()->emit_return( ret_v, service()->param_abi(false) );
			} else {
				EFLIB_ASSERT_UNIMPLEMENTED();
			}
		}
		else if( intr->unmangled_name() == "asint" || intr->unmangled_name() == "asfloat" || intr->unmangled_name() == "asuint" )
		{
			function_t& fn = service()->fn();
			fn.arg_name(0, "v");
			value_tyinfo* ret_ty = fn.get_return_ty().get();
			service()->emit_return( service()->cast_bits(fn.arg(0), ret_ty), service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "fmod" )
		{
			function_t& fn = service()->fn();

			assert( fn.arg_size() == 2 );
			fn.arg_name(0, "lhs");
			fn.arg_name(1, "rhs");
			
			service()->emit_return( service()->emit_mod( fn.arg(0), fn.arg(1) ), service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "radians" )
		{
			function_t& fn = service()->fn();
			assert( fn.arg_size() == 1 );
			fn.arg_name(0, ".deg");
			float deg2rad = (float)(eflib::PI/180.0f);
			value_t deg2rad_scalar_v = service()->create_constant_scalar(deg2rad, NULL, builtin_types::_float);
			value_t deg2rad_v = service()->create_value_by_scalar( deg2rad_scalar_v, fn.arg(0).tyinfo(), fn.arg(0).tyinfo()->hint() );
			service()->emit_return( service()->emit_mul_comp( deg2rad_v, fn.arg(0) ), service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "degrees" )
		{
			function_t& fn = service()->fn();
			assert( fn.arg_size() == 1 );
			fn.arg_name(0, ".rad");
			float rad2deg = (float)(180.0f/eflib::PI);
			value_t rad2deg_scalar_v = service()->create_constant_scalar(rad2deg, NULL, builtin_types::_float);
			value_t rad2deg_v = service()->create_value_by_scalar( rad2deg_scalar_v, fn.arg(0).tyinfo(), fn.arg(0).tyinfo()->hint() );
			service()->emit_return( service()->emit_mul_comp( rad2deg_v, fn.arg(0) ), service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "lerp" )
		{
			function_t& fn = service()->fn();
			assert( fn.arg_size() == 3 );
			fn.arg_name(0, ".s");
			fn.arg_name(1, ".d");
			fn.arg_name(2, ".t");
			value_t diff = service()->emit_sub( fn.arg(1), fn.arg(0) );
			value_t t_diff = service()->emit_mul_comp( diff, fn.arg(2) );
			value_t ret = service()->emit_add(fn.arg(0), t_diff);
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "distance" )
		{
			function_t& fn = service()->fn();
			assert( fn.arg_size() == 2 );
			fn.arg_name(0, ".s");
			fn.arg_name(1, ".d");
			value_t diff = service()->emit_sub( fn.arg(1), fn.arg(0) );
			value_t dist_sqr = service()->emit_dot(diff, diff);
			value_t dist = service()->emit_sqrt(dist_sqr);
			service()->emit_return( dist, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "dst" )
		{
			function_t& fn = service()->fn();
			assert( fn.arg_size() == 2 );
			fn.arg_name(0, ".sqr");
			fn.arg_name(1, ".inv");
			value_t x2 = service()->create_constant_scalar(1.0f, NULL, builtin_types::_float);
			value_t y0 = service()->emit_extract_val( fn.arg(0), 1 );
			value_t y1 = service()->emit_extract_val( fn.arg(1), 1 );
			value_t z0 = service()->emit_extract_val( fn.arg(0), 2 );
			value_t w1 = service()->emit_extract_val( fn.arg(1), 3 );
			value_t y2 = service()->emit_mul_comp( y0, y1 );
			vector<value_t> elems;
			elems.push_back(x2);
			elems.push_back(y2);
			elems.push_back(z0);
			elems.push_back(w1);
			value_t dest = service()->create_vector(elems, service()->param_abi(false) );
			service()->emit_return( dest, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "any" )
		{
			function_t& fn = service()->fn();
			assert( fn.arg_size() == 1 );
			fn.arg_name(0, ".v");
			service()->emit_return( service()->emit_any(fn.arg(0)), service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "all" )
		{
			function_t& fn = service()->fn();
			assert( fn.arg_size() == 1 );
			fn.arg_name(0, ".v");
			service()->emit_return( service()->emit_all(fn.arg(0)), service()->param_abi(false) );
		}
		else
		{
			EFLIB_ASSERT( !"Unprocessed intrinsic.", intr->unmangled_name().c_str() );
		}
		service()->clean_empty_blocks();
	}
}

cgllvm_sctxt const * sc_ptr( const boost::any& any_val ){
	return any_cast<cgllvm_sctxt>(&any_val);
}

cgllvm_sctxt* sc_ptr( boost::any& any_val ){
	return any_cast<cgllvm_sctxt>(&any_val);
}

cgllvm_sctxt const * sc_ptr( const boost::any* any_val )
{
	return any_cast<cgllvm_sctxt>(any_val);
}

cgllvm_sctxt* sc_ptr( boost::any* any_val )
{
	return any_cast<cgllvm_sctxt>(any_val);
}

cgllvm_sctxt_data* sc_data_ptr( boost::any* any_val ){
	return addressof( sc_ptr(any_val)->data() );
}

cgllvm_sctxt_data const* sc_data_ptr( boost::any const* any_val ){
	return addressof( sc_ptr(any_val)->data() );
}

cgllvm_sctxt_env* sc_env_ptr( boost::any* any_val ){
	return addressof( sc_ptr(any_val)->env() );
}

cgllvm_sctxt_env const* sc_env_ptr( boost::any const* any_val ){
	return addressof( sc_ptr(any_val)->env() );
}

END_NS_SASL_CODE_GENERATOR();
