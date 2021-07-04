# PgSQL_IntSet
UNSW COMP9315 Project, customer data type for PostgreSql, write by C

## Background

PostgreSQL has an extensibility model which, among other things, provides a well-defined process for adding new data types into a PostgreSQL server. 
This capability has led to the development by PostgreSQL users of a number of types (such as polygons) which have become part of the standard distribution. 
It also means that PostgreSQL is the database of choice in research projects which aim to push the boundaries of what kind of data a DBMS can manage.

In this project, we will add a new data type for dealing with **sets of integers**.


## Setting Up
We need a fresh copy of PostgreSQL source code to set up this project. Note that you only need to configure, compile, and install your PostgreSQL server once for this assignment. All subsequent compilation takes place in the `src/tutorial` directory. 

Download PostgreSQL (recommend v12.5 for this project) source code from [here](https://www.postgresql.org/ftp/source/v12.5/).
Once you installed your PostgreSQL server, you should go to `where\your\PG_CODE\src\tutorial` and copy `intsert.c` and `intset.source` to this dictionary.
And once you've made the intset files, you should also edit the Makefile in this directory, change and add text _`intset`_ and _`intset.sql`_ to the following lines:
```
MODULES = complex funcs intset
DATA_built = advanced.sql basics.sql complex.sql funcs.sql syscat.sql intset.sql
```
Then run `make` and apply `intset.sql` to your PostgreSQL server, remember to remove this intset type by `DROP TYPE complex CASCADE;` when you finish using this new data type.


## The intSet Data Type

We aim to define a new base type intSet, to store the notion of sets of integer values. 
We also aim to define a useful collection of operations on the **intSet** type.

### intSet values

In mathematics, we represent a set as a curly-bracketed, comma-separated collection of values: **{1,2,3,4,5}**
 
Such a set contains only distinct values, and no particular ordering can be imposed.

Our intSet values can be represented similarly. 
We can have a comma-separated list of non-negative integers, surrounded by a set of curly braces, which is presented to and by PostgreSQL as a string. 
For example: 
`'{ 1, 2, 3, 4, 5 }'`.

Whitespace is not matter, so `'{1,2,3}'` and `'{ 1, 2, 3 }'` are equivalent. 
Similarly, a set contains distinct values, so `'{1,1,1,1,1}' `is equivalent to `'{1}'`. 
And ordering is irrelevant, so `'{1,2,3}'` is equivalent to `'{3,2,1}'`.

The integer values in the set are assumed to consist of a sequence of digits. There are no + or - signs. 
There can be leading zeroes, but they should effectively be ignored, e.g. `0001` should be treated the same as `1`.

### Operations on intSets

We have following operations for the `intSet` type: 
| Entity   | Description                             |
| ------   | --------------------------------------- |
| i ? S    | intSet _`S`_ contains the intger _`i`_; That is, **`i∈S`** |
|   # S    | Give the cardinality, or number of distinct elements in, intSet _`S`_; That is, **`\|S\|`** |

