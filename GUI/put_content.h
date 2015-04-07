/*
Upload files on server.
*/
void put_content(char *arg,char *user_input,Appstate *app_state)
{	
	
	/* Temporary variables*/
	int no_of_bytes;
	int port;	
	int newsockfd;
	int fd;	
	int p;
	int total;
	int size;
	
	struct timeval tm;/* time structure to set time wait for receive buffer */
	tm.tv_sec = 1;
	tm.tv_usec = 750000;
	
	struct stat file_buff;

	char message_from_server[MAXSZ];
	char message_to_server[MAXSZ];
	char file[MAXSZ];// File name
	char file_name[MAXSZ];// File name with instruction to server
	char data[MAXSZ];// Data transfer

	/* Initialise all the character arrays */
	bzero(message_from_server,MAXSZ);
	bzero(message_to_server,MAXSZ);
	bzero(file_name,MAXSZ);
	bzero(file,MAXSZ);
	bzero(data,MAXSZ);
	
	/* Tell server to change to BINARY mode */
	send(app_state->sockfd,"TYPE I\r\n",8,0);
	sprintf(buff,"Command: TYPE I\nResponse: ");
	print_buff(app_state);
	
	while((no_of_bytes = recv(app_state->sockfd,message_from_server,MAXSZ,0)) > 0)
	{
		message_from_server[no_of_bytes] = '\0';
		sprintf(buff,"%s",message_from_server);
		print_buff(app_state);
		
		if(strstr(message_from_server,"200 ") > 0 || strstr(message_from_server,"501 ") > 0 ||strstr(message_from_server,"500 ") > 0 ||strstr(message_from_server,"504 ") > 0 ||strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"530 ") > 0)
			break;	
	}
	sprintf(buff,"\n");
	print_buff(app_state);
	
	if(strstr(message_from_server,"501 ") > 0 ||strstr(message_from_server,"500 ") > 0 ||strstr(message_from_server,"504 ") > 0 ||strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"530 ") > 0)
		return;	

	/* Send request for PASSIVE connection */	
	send(app_state->sockfd,passive,strlen(passive),0);
	sprintf(buff,"Command: %sResponse: ",passive);
	print_buff(app_state);
	
	while((no_of_bytes = recv(app_state->sockfd,message_from_server,MAXSZ,0)) > 0)
	{
		message_from_server[no_of_bytes] = '\0';
		sprintf(buff,"%s",message_from_server);
	
		print_buff(app_state);
		
		if(strstr(message_from_server,"227 ") > 0 || strstr(message_from_server,"501 ") > 0 ||strstr(message_from_server,"500 ") > 0 ||strstr(message_from_server,"502 ") > 0 ||strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"530 ") > 0)
			break;
	
	}
	sprintf(buff,"\n");
	print_buff(app_state);

	if(strstr(message_from_server,"501 ") > 0 ||strstr(message_from_server,"500 ") > 0 ||strstr(message_from_server,"502 ") > 0 ||strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"530 ") > 0)
		return;

	/* Server accepts request and sends PORT variables */
	if(strncmp(message_from_server,"227",3) == 0)
	{
		/* Generate a PORT number using PORT variables */
		port = passive_port_number(message_from_server); 
	
		/* Connect to server using another PORT for file transfers */
		newsockfd = func_to_connect_passive(arg,port);
		fcntl(newsockfd,F_SETFL,FNDELAY);
		
		/* Send file name to server */
		sprintf(file_name,"STOR %s\r\n",user_input + 4);	
		sprintf(buff,"Command: STOR %s\nResponse: ",user_input + 4);	
		
		print_buff(app_state);
		send(app_state->sockfd,file_name,strlen(file_name),0);
			
		while((no_of_bytes = recv(app_state->sockfd,message_from_server,MAXSZ,0)) > 0)
		{
			message_from_server[no_of_bytes] = '\0';
			sprintf(buff,"%s",message_from_server);
			print_buff(app_state);
			
			if(strstr(message_from_server,"550 ") > 0 ||strstr(message_from_server,"125 ") > 0 ||strstr(message_from_server,"150 ") > 0 || strstr(message_from_server,"501 ") > 0 ||strstr(message_from_server,"500 ") > 0 ||strstr(message_from_server,"452 ") > 0 ||strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"530 ") > 0 || strstr(message_from_server,"553 ") > 0 ||strstr(message_from_server,"532 ") > 0)
				break;
		}
		sprintf(buff,"\n");
		print_buff(app_state);
	
		/* Send file data to server */
		if(strncmp(message_from_server,"150",3) == 0 || strncmp(message_from_server,"125",3) == 0)
		{
			sprintf(file,"%s",user_input + 4);

			fd = open(file,O_RDONLY);
			fstat(fd,&file_buff);
			size = (int)file_buff.st_size;
			while(size > 0)
			{
				no_of_bytes = read(fd,data,MAXSZ);
				total = 0;
				while(total < no_of_bytes)
                		{
					while(gtk_events_pending())
						gtk_main_iteration();
                		
				        p = send(newsockfd,data + total,no_of_bytes - total,0);
                       			 total += p;
				}
				size -= no_of_bytes;
			}
					
			close(newsockfd);
			/* Set time boundation on receive buffer */
			setsockopt(app_state->sockfd, SOL_SOCKET, SO_RCVTIMEO,(char *)&tm,sizeof(tm));
			while((no_of_bytes = recv(app_state->sockfd,message_from_server,MAXSZ,0)) > 0)
			{
				message_from_server[no_of_bytes] = '\0';
				sprintf(buff,"%s",message_from_server);
				print_buff(app_state);
				if(strstr(message_from_server,"226 ") > 0)
					break;
			}
			sprintf(buff,"\n");
			print_buff(app_state);
		}
				
	}
	return;
}
