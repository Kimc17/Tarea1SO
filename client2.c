#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>

int send_image(int socket, const char *ruta)
{

  FILE *picture;
  int size, read_size, stat, packet_index;
  char send_buffer[10240], read_buffer[256];
  packet_index = 1;

  picture = fopen(ruta, "r");
  printf("Getting Picture Size\n");

  if (picture == NULL)
  {
    printf("Error Opening Image File");
  }

  fseek(picture, 0, SEEK_END);
  size = ftell(picture);
  fseek(picture, 0, SEEK_SET);
  printf("Total Picture size: %i\n", size);

  //Send Picture Size
  printf("Sending Picture Size\n");
  write(socket, (void *)&size, sizeof(int));

  //Send Picture as Byte Array
  printf("Sending Picture as Byte Array\n");

  do
  { //Read while we get errors that are due to signals.
    stat = read(socket, &read_buffer, 255);
    printf("Bytes read: %i\n", stat);
  } while (stat < 0);

  printf("Received data in socket\n");
  printf("Socket data: %s\n", read_buffer);

  while (!feof(picture))
  {
    //while(packet_index = 1){
    //Read from the file into our send buffer
    read_size = fread(send_buffer, 1, sizeof(send_buffer) - 1, picture);

    //Send data through our socket
    do
    {
      stat = write(socket, send_buffer, read_size);
    } while (stat < 0);

    printf("Packet Number: %i\n", packet_index);
    printf("Packet Size Sent: %i\n", read_size);
    printf(" \n");
    printf(" \n");

    packet_index++;

    //Zero out our send buffer
    bzero(send_buffer, sizeof(send_buffer));
  }
}

int main(int argc, char *argv[])
{

  int socket_desc;
  int fin = 0;
  struct sockaddr_in server;
  char *parray;

  //Create socket
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc == -1)
  {
    printf("Could not create socket");
  }

  memset(&server, 0, sizeof(server));
  server.sin_addr.s_addr = inet_addr(argv[1]);
  server.sin_family = AF_INET;
  server.sin_port = htons(8082);

  //Connect to remote server
  if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    puts(strerror(errno));
    close(socket_desc);
    puts("Connect Error");
    return 1;
  }
  printf("La IP es: %s \n", argv[1]);
  puts("Connected\n");
  char ruta[30];
  char *ret;
  while (fin != 1)
  {
    printf("Escriba la ruta de la imagen: ");
    scanf("%s", ruta);
    ret = strstr(ruta, "fin");

    if (ret != NULL)
    {
      printf("Finalizando aplicación...");
      close(socket_desc);
      // fin = 1;
      return 0;
    }
    else
    {
      printf("Enviando: %s \n", ruta);
      send_image(socket_desc, ruta);
    }
  }
}