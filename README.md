# PgSQL_IntSet
UNSW COMP9315 Project, customer data type for PostgreSql

## Background

PostgreSQL has an extensibility model which, among other things, provides a well-defined process for adding new data types into a PostgreSQL server. 
This capability has led to the development by PostgreSQL users of a number of types (such as polygons) which have become part of the standard distribution. 
It also means that PostgreSQL is the database of choice in research projects which aim to push the boundaries of what kind of data a DBMS can manage.

In this project, we will add a new data type for dealing with **sets of integers**.


## Setting Up
We need a fresh copy of PostgreSQL source code to set up this project. Note that you only need to configure, compile, and install your PostgreSQL server once for this assignment. All subsequent compilation takes place in the `src/tutorial` directory. 