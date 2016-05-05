//
// CS 431 - Lab 03 Server Skeleton
// PC/Linux (Provided)
//

#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "pc_crc16.h"
#include "lab03.h"

int main(int argc, char* argv[])
{
  double troll_pct=0;// Perturbation % for the troll (if needed)
  int ifd,ofd,i,N,troll=0;// Input and Output file descriptors (serial/troll)
  char str[MSG_BYTES_MSG],opt;// String input
  char c;
  int ack;
  int cnt = 0;
  int mCrc,attempts=0;
  int errno = -1;
  struct termios oldtio, tio;// Serial configuration parameters
  int VERBOSE = 0;// Verbose output - can be overriden with -v

  // Command line options
  while ((opt = getopt(argc, argv, "t:v")) != -1) {
    switch (opt) {
    case 't':troll = 1; 
      troll_pct = atof(optarg);
      break;
    case 'v':VERBOSE = 1; break;
    default: break;
    }
  }

  printf("CS431 - Lab 03 Server\n(Enter a mssg to send.  Type \"quit\" to exit)\n");


  //
  // WRITE ME: Open the serial port (/dev/ttyS0) read-write
  //
  ifd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);// | O_NONBLOCK );//O_RDWR);
  if (ifd == -1){
    printf("Error Reading/Writing TTYS0");
    return -1;
  }
  //printf("file descriptor opened\n");


  // Start the troll if necessary
  if (troll)
    {
      // Open troll process (lab03_troll) for output only
      FILE * pfile;// Process FILE for troll (used locally only)
      char cmd[128];// Shell command

      snprintf(cmd, 128, "./lab03_troll -p%f %s", troll_pct, (VERBOSE) ? "-v" : "");

      pfile = popen(cmd, "w");
      if (!pfile) { perror("lab03_troll"); exit(-1); }
      ofd = fileno(pfile);
    }
  else ofd = ifd;// Use the serial port for both input and output
  //printf("done trolling\n");


  //
  // WRITE ME: Set up the serial port parameters and data format
  //
  tcgetattr(ifd, &oldtio); // read default config
  //memcpy ( &tio, &oldtio, sizeof(struct termios) );
 
  // change config here
  //tio.c_cflag = (tio.c_cflag & ~CSIZE) | CS8;
  //tio.c_cflag |= B9600;
  //tio.c_cflag &= ~(PARENB | PARODD);      // shut off parity
  //tio.c_cflag &= ~CSTOPB;
  //tcgetattr(ifd, &oldtio);
  tio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
  tio.c_iflag = 0;
  tio.c_oflag = 0;
  tio.c_lflag = 0;
  tcflush(ifd, TCIFLUSH);
  tcsetattr(ifd, TCSANOW, &tio);
 
  /*if (tcsetattr (ifd, TCSANOW, &tio) != 0)
    {
        fprintf (stderr, "error %d from tcsetattr", errno);
	    return -1;
	    }*/
  //  printf("new config set\n");
  //close(ifd);

  char mssg[260];
  while(1)
    {

      //
      // WRITE ME: Read a line of input (Hint: use fgetc(stdin) to read each 
      while (1){
	c = fgetc(stdin);
	if (c == '\n')
	  break;
	str[cnt++] = c;
      }
      str[cnt] = NULL;
      //printf("%s\n",str);
      //printf("%d\n",cnt);
      if (strcmp(str, "quit") == 0) break;

      //
      // WRITE ME: Compute crc (only lowest 16 bits are returned)
      //
        
      cnt = strlen(str);
      mCrc = pc_crc16(str, cnt);
      ack = 0, attempts = 0;

      mssg[0] = 0x0;
      mssg[1] = (mCrc & 0xff00) >> 8;
      mssg[2] = mCrc & 0xff;
      mssg[3] = cnt;
      strcpy(&(mssg[4]), str);
      //printf("%s\n", mssg);
      printf("%x\n", mCrc);
      
      while (!ack)
        {

	  printf("Sending (attempt %d)...\n", ++attempts);
	  
	  //ofd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NONBLOCK);//O_RDWR);
	  if (ofd == -1){
	    printf("Error Reading/Writing TTYS0");
	    return -1;
	  }
	  write(ofd, &mssg, 4+cnt);
	  //close(ofd);
	  printf("Message sent, waiting for ack... ");

	  
	  //ifd = open("/dev/ttyS0", O_RDWR);
	  //
	  // WRITE ME: Wait for MSG_ACK or MSG_NACK
	  //
	  //fflush(ifd);
	  while(read(ifd,&ack,1) < 0);
	  //fprintf (stderr, "error %d from read\n", errno);
	  //close(ifd);
	  //printf("bytes read ack: %d\n", read(ifd,&ack,4));
	  //fgets(ack, ifd);
	  //printf("ack is %x\n",ack);
	  //ack = 1;
	  printf("%s\n", ack ? "ACK" : "NACK, resending");
	}
      cnt = 0;
      ack = 0;
      printf("\n");
    }



  //
  // WRITE ME: Reset the serial port parameters
  //
  if (tcsetattr (ifd, TCSANOW, &oldtio) != 0)
    {
      fprintf (stderr, "error %d from tcsetattr", errno);
      return -1;
    }


  // Close the serial port
  close(ifd);
  
  return 0;
}
