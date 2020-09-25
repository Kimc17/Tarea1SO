#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <magick/api.h>
#include <signal.h>
int foo = 1;
void catch (int sig)
{
  printf("Señal: %d atrapada!\n", sig);
  foo = 0;
}
int DominantColor(char const *name)
{
  ExceptionInfo
      exception;

  Image
      *images;

  ImageInfo
      *image_info;

  /*
        Initialize the image info structure and read an image.
      */
  GetExceptionInfo(&exception);
  image_info = CloneImageInfo((ImageInfo *)NULL);
  (void)strcpy(image_info->filename, name);
  images = ReadImage(image_info, &exception);
  if (exception.severity != UndefinedException)
    CatchException(&exception);
  if (images == (Image *)NULL)
    exit(1);
  /*
        Turn the images into a thumbnail sequence.
      */

  FILE *fptr;

  fptr = fopen("prueba.txt", "w");

  if (fptr == NULL)
  {
    printf("Error!");
    exit(1);
  }
  GetNumberColors(images, fptr, &exception);
  fclose(fptr);

  FILE *archivo;
  char caracteres[100];
  archivo = fopen("prueba.txt", "r");
  int R = 0;
  int G = 0;
  int B = 0;
  while (feof(archivo) == 0)
  {

    fgets(caracteres, 100, archivo);
    char *p;
    p = caracteres;
    int i = 0;
    int comas = 0;
    int rojo = 0;
    int azul = 0;
    int verde = 0;
    while (*p != '\0')
    {
      if (*p == '(')
      {
        i = 1;
      }
      else if (*p == ')')
      {
        i = 0;
      }
      if (i == 1)
      {
        if (*p == ',')
        {
          comas++;
        }
        else if (*p != ' ' && *p != '(')
        {
          int num = *p - '0';
          if (comas == 0)
          {
            rojo = rojo * 10 + num;
          }
          else if (comas == 1)
          {
            verde = verde * 10 + num;
          }
          else if (comas == 2)
          {
            azul = azul * 10 + num;
          }
        }
      }
      p++;
    }
    //printf("The substring is: %s\n", ret);
    R += rojo;
    G += verde;
    B += azul;
  }
  printf("%d \n", R);
  printf("%d \n", G);
  printf("%d \n", B);
  fclose(archivo);
  DestroyImageInfo(image_info);
  DestroyExceptionInfo(&exception);
  DestroyMagick();
  if (R > G && R > B)
  {
    return 0;
  }
  else if (G > R && G > B)
  {
    return 1;
  }
  else
  {
    return 2;
  }
}

int receive_image(int socket, int env, char *argv[])
{ // Start function
  InitializeMagick(*argv);
  int recv_size = 0, size = 0, read_size, write_size, packet_index = 1, stat;

  char imagearray[10241];
  FILE *image;

  //Find the size of the image
  do
  {
    stat = read(socket, &size, sizeof(int));
  } while (stat < 0);

  printf("Packet received.\n");
  printf("Packet size: %i\n", stat);
  printf("Image size: %i\n", size);
  printf(" \n");

  char buffer[] = "Got it";
  //Send our verification signal
  do
  {
    stat = write(socket, &buffer, sizeof(int));
  } while (stat < 0);

  printf("Reply sent\n");
  printf(" \n");
  char num2[10] = "";
  sprintf(num2, "%d.png", env);
  image = fopen(num2, "w");

  if (image == NULL)
  {
    printf("Error has occurred. Image file could not be opened\n");
    return -1;
  }

  //Loop while we have not received the entire file yet

  struct timeval timeout = {10, 0};

  fd_set fds;
  int buffer_fd;

  while (recv_size < size)
  {
    //while(packet_index < 2){

    FD_ZERO(&fds);
    FD_SET(socket, &fds);

    buffer_fd = select(FD_SETSIZE, &fds, NULL, NULL, &timeout);

    if (buffer_fd < 0)
      printf("error: bad file descriptor set.\n");

    if (buffer_fd == 0)
      printf("error: buffer read timeout expired.\n");

    if (buffer_fd > 0)
    {
      do
      {
        read_size = read(socket, imagearray, 10241);
      } while (read_size < 0);

      printf("Packet number received: %i\n", packet_index);
      printf("Packet size: %i\n", read_size);

      //Write the currently read data into our image file
      write_size = fwrite(imagearray, 1, read_size, image);
      printf("Written image size: %i\n", write_size);

      if (read_size != write_size)
      {
        printf("error in read write\n");
      }

      //Increment the total number of bytes read
      recv_size += read_size;
      packet_index++;
      printf("Total received image size: %i\n", recv_size);
      printf(" \n");
    }
  }

  fclose(image);
  printf("¡Imagen recibida!\n");
  int pred = DominantColor(num2);
  printf("El color predominante es %d \n", pred);

  char num[50] = "";
  if (pred == 0)
  {
    sprintf(num, "volumen/R/%d.png", env);
  }
  else if (pred == 1)
  {
    sprintf(num, "volumen/G/%d.png", env);
  }
  else
  {
    sprintf(num, "volumen/B/%d.png", env);
  }

  printf("El nombre será %s \n", num);

  FILE *file1, *file2;
  int data1 = 0;

  file1 = fopen(num2, "r");
  file2 = fopen(num, "w");

  while ((data1 = fgetc(file1)) != EOF)
  {
    fputc(data1, file2);
  }

  fclose(file1);
  fclose(file2);
  remove(num2);

  return 1;
}

int main(int argc, char *argv[])
{

  int socket_desc, new_socket, c;
  struct sockaddr_in server, client;
  //Create socket
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_desc == -1)
  {
    printf("Could not create socket");
  }

  //Prepare the sockaddr_in structure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(8082);

  //Bind
  if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    puts("bind failed");
    return 1;
  }

  puts("bind done");

  //Listen
  listen(socket_desc, 3);

  //Accept and incoming connection
  puts("Esperando conexiones...");
  c = sizeof(struct sockaddr_in);

  if ((new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c)))
  {
    puts("Conexión aceptada");
  }

  fflush(stdout);

  if (new_socket < 0)
  {
    perror("Falló la conexión");
    return 1;
  }
  int env = 0;
  signal(SIGINT, &catch);

  struct in_addr clientIp = client.sin_addr;
  char ipStr[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &clientIp, ipStr, INET_ADDRSTRLEN);
  printf("Para ver algo %s \n", ipStr);

  FILE *archivo2;
  char caracteres[100];
  archivo2 = fopen("volumen/configuracion.config", "r");
  char *ret;
  while (feof(archivo2) == 0)
  {
    fgets(caracteres, 100, archivo2);
    printf("%s", caracteres);
    ret = strstr(caracteres, ipStr);
    if (ret != NULL)
    {
      puts("Sí lo leí");
      while (foo)
      {
        receive_image(new_socket, env, argv);
        fflush(stdout);
        env++;
      }
      return 0;
    }
    else
    {
    }
  }
  puts("La conexión no fue autorizada");
  fclose(archivo2);
  close(socket_desc);
  return 0;
}