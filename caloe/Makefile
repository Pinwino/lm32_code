 # /**
 # ******************************************************************************* 
 # * @file Makefile
 # *  @brief Caloe Project Makefile
 # *
 # *  Copyright (C) 2013
 # *
 # *  @author Miguel Jimenez Lopez <klyone@ugr.es>
 # *
 # *  @bug ---
 # *
 # *******************************************************************************
 # *  This library is free software; you can redistribute it and/or
 # *  modify it under the terms of the GNU Lesser General Public
 # *  License as published by the Free Software Foundation; either
 # *  version 3 of the License, or (at your option) any later version.
 # *
 # *  This library is distributed in the hope that it will be useful,
 # *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 # *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 # *  Lesser General Public License for more details.
 # *  
 # *  You should have received a copy of the GNU Lesser General Public
 # *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 # *******************************************************************************
 
all: tst.run

access_internals.o: access_internals.h access_internals.c
	@echo "lib: Compiling access_internals..."
	@gcc -o access_internals.o -c access_internals.c
	
tst.o: tst.c
	@echo "tools: Compiling tst object..."
	@gcc -g -c -o tst.o tst.c

tst.run: tst.o access_internals.o ../../wr/etherbone-core/api/libetherbone.a
	@echo "tools: Compiling tst..."
	@gcc -g -o tst.run tst.o access_internals.o -l:../../wr/etherbone-core/api/libetherbone.a
		

clean:
	@echo "lib: Cleanup..."
	@-rm *.o *~ *.a *.run
