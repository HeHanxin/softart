@echo Counting code...
@"./code_statistic.py" ../../softart .h;.cpp;.py;.xgt;.xgp;.txt;.cmake > codelines_of_softart.txt
@"./code_statistic.py" ../../sasl .h;.cpp;.py;.xgt;.xgp;.txt;.cmake >> codelines_of_softart.txt
@echo Code counting finished.
@echo off
pause