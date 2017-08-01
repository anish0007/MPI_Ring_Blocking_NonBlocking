//Elaheh Rashedi

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
        int pid, next, prev, message, tag = 1 , p ;
        float Input;
        float total;

        /* Start up MPI */

        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &pid);
        MPI_Comm_size(MPI_COMM_WORLD, &p);

	// the requests should be checked in non blocking send and receive
        MPI_Request req1 , req2 , req3 , req4 ;
	MPI_Status status;

        // Calculate the rank of the next process in the ring.

        next = pid + 1 ;
        prev = pid - 1 ;
        if (pid == (p-1)) next = 0 ;
        if (pid == 0 )    prev = p-1 ;

	// process p0 read the file for the first time
        if ( pid == 0 )
        {
                float num;
                FILE *in = fopen("data.txt","r");  // read inputs from data2.txt input file
                fscanf ( in , "%f", & num );
                Input = num;
                fclose (in);
                printf("The value %f is read from file \n", Input );
                //MPI_Send(&Input, 1, MPI_FLOAT, 1 , tag, MPI_COMM_WORLD); // send to p0
                MPI_Isend(&Input, 1, MPI_FLOAT, 1 , tag, MPI_COMM_WORLD , &req1 ); // send to p0

		MPI_Wait(&req1, &status);

        }

        int flag = 1 ;

	// all of the processors do this part
	// they are sending and receiving in a loop
	// when we reache to the threashold , the while loop will be breaked
        while ( flag == 1 ) {

                printf("P%d is waiting to receive data \n", pid );
                // receive data from previous processor\
		// or receive -1 from any of the processors
                //MPI_Recv(&Input, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG , MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Irecv(&Input, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG , MPI_COMM_WORLD, &req2 );

		// we should wait for receive to be completed
		MPI_Wait(&req2, &status);

                if (Input >= 0.01)
                {
                        Input = Input * 0.95;
			// now if this is not threashold , we should continue on sending
                        //MPI_Send(&Input, 1, MPI_FLOAT, next, tag, MPI_COMM_WORLD);
                        MPI_Isend(&Input, 1, MPI_FLOAT, next, tag, MPI_COMM_WORLD , &req3 );
                        printf("P%d sent the value %f to p%d \n", pid , Input , next);
			
			// we can wait on continue to be completed
			MPI_Wait(&req3, &status);

                }
                else if (Input >= 0 )
                {
                        printf("The value %f is received in p%d : LESS THAN THREASHOLD \n", Input , pid );
                        printf(" ***** P%d EXIT ***** \n", pid );
                        Input = -1 ;

                        // we can not broad cast the data here
			// because the other processors may not reach to this part
			// and they may loos the broadcast data
                        //MPI_Bcast ( &Input, 1 , MPI_INT , pid , MPI_COMM_WORLD ) ;

			// here we send -1 to all processors
                        for (int i =0 ; i < p ; i++)
                                if (i != pid){
                                        //MPI_Send(&Input, 1, MPI_FLOAT, i , tag, MPI_COMM_WORLD);
                                        MPI_Isend(&Input, 1, MPI_FLOAT, i , tag, MPI_COMM_WORLD , &req4);
                                        printf("P%d sent the value -1 to p%d \n", pid , i );

					MPI_Wait(&req4, &status);
                                }

                        flag = 0 ; //exit

                }
                else // -1
                {
			// it means that the loop has been ended and we should break the loop and exit
			
                        printf("P%d is received the value -1. \n", pid );
                        printf(" ***** P%d EXIT ***** \n", pid );
                        flag = 0 ; //exit
                }


        }

 
        // finilizing the process

        MPI_Finalize();
        return 0;

}
           

