/**
 * @author  Mitchel Taylor <mitchelt@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "../include/router.h"

/**
 * main function
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
using namespace std;
int main(int argc, char **argv)
{

	if(argc < 2){
		cout << "Error: Not Enough Inputs\n";
		exit(-1);
	}else if(argc > 2){
		cout << "Error: Too Many Inputs\n";
		exit(-1);
	}else{
		
		// Convert to integer
		//cout << "Attemping to start router with control port " << argv[1] << "\n";
		int control_port = atoi(argv[1]);
		router r;
		//cout << "Starting Router\n";
		r.startRouter(control_port);

	}






	
	return 0;
}
