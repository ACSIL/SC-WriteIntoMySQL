# Sierra Chart ASCIL study writing entries into MySQL

Couple of functions for writing entries into MySQL when new position is opened and closed. 

It is quite a tricky thing to set up Visual Studio C++ and Sierra Chart ACSIL with MySQL database so here are a few words on that.

The project is set up in such a way that it should reflect all the paths and libs that VS needs to work with MySQL 8. You dont need to do manually add anything, just clone it, and build.

## What you need:

You need to have installed SierraChart obviously. The default path where SierraCharts installs itself is on C:\SierraChart. You can have multiple instances of SierraChart on your machine and this project is setup to work with just with that. Instead of default C:\SierraChart i use C:\SieraChartVS path. So basically the only thing you need to do is to install a new instance of SierraChart and tell it to be install on C:\SierraChartVS. That is all and it will work just fine, after you build, you will see the study in C:\SierraChartVS\Data folder. It will be called WRITE INTO MYSQL.dll (this is qhat you put into SierraChar via add study).

You need to have installed MySQL on your pc and CPP connector 1.1 (it is done during instalation process when installing MySQL). The setting respects the default paths of where MySQL install things, so if you dont change anything during instalation of MySQL, you will be just fine. In order to MySQL work properly you need to add three additional dependencies into the SierraChart data folder - the target place where the study .dll will go after it is build.

These three dependecies are all in C:\Program Files\MySQL\MySQL Server 8.0\bin
1] libeay32.dll
2] libmysql.dll
3] ssleay32
just copy them all from C:\Program Files\MySQL\MySQL Server 8.0\bin to C:\SierraChartVS\Data 

## Important notice:
Be sure you build it as Release - 64bit project - https://prnt.sc/sd6d1p. If you try to buil in on Debug, it will not work and you will see a linker error.
Be sure you run your SierraChart 64bit, otherwise you will not see this study and will not able to add it.

## TODO:
Refactor the code. Now it is all messed up in one place. I focused on make-it-work rather then make it nice. For example the query build should be separated etc.
