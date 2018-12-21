/*
 Montasir Bechi Nasir
 *   

The following program is an HTTP Proxy. This program is a silly program that takes the web page you requested and inserts
random errors in it, and then displays the contents on your browser. 

To use the program, simply compile the program you g++, and execute the file. This program takes no command line arguments.
The following program works requires you to set your port number to 12345 on your browser. Once your browser is opened, you can
simply take a web page url and paste it into a browser of your choice (I used chrome/mozilla for testing) and enter the url of 
the site you would like. The program has been tested and returns an html page with errors when the html file contains body
tags, otherwise it will just pass back the content without inserting anything to the payload. Additionally, this file now 
places random errors in bold in html files. Moreover, this program places random errors in plaintext files as well.
Lastly, the program sends back gif/jpeg images successfully without making changes to them.
compile:
g++ proxyServer.cpp
 */

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <string.h>

#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <string>
#include <iostream>
#include <vector>
using namespace std;

 
// function declaration
string getFromWebServer(string something, char ** pointer);
int isGETRequest(string req);
string getHostNameFromReq(string getRequestString);
string getDirectoryFromReq(string getRequestString, string host_str);
int isOKAndHTMLOrText(string responseFromServer);
string insertRandomErrors(std::string responseFromServer);


int main() {

	/* Address initialization */
	struct sockaddr_in server;
	int MYPORTNUM = 12345;
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(MYPORTNUM);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	/* Create the listening socket */
	int lstn_sock;
	lstn_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (lstn_sock < 0) {
		printf("Error in socket() while creating lstn_sock\n");
		exit(-1);
	}
	printf("Socket created successfully.\n");

	/* Bind the socket to address and port */
	int status;
	status = bind(lstn_sock, (struct sockaddr *) &server,
			sizeof(struct sockaddr_in));
	if (status < 0) {
		printf("Error in bind()\n");
		exit(-1);
	}
	printf("Binding completed.\n");

	/* Connect to TCP server */
	status = listen(lstn_sock, 5);
	if (status < 0) {
		printf("Error in listen()\n");
		exit(-1);
	}
	printf("Listening for connection requests...\n");

	/* Main Loop for listening */
	while (1) {

		/* Accept a connection */
		int connected_sock;
		connected_sock = accept(lstn_sock, NULL,
		NULL);
		if (connected_sock < 0) {
			printf("Error in accept()\n");
			exit(-1);
		}
		printf("Connection established.\n");

		/* Send data*/
		int count;
		char message[1024] = { "For termination send \"Bye\"\n" };
		//count = send(connected_sock, message, sizeof(message), 0);
		//if (count < 0) {
	//		printf("Error in send()\n");
	//	}

		/* Receive data */
		char rcv_message[52000];
		while (1) {
			count = recv(connected_sock, rcv_message, sizeof(rcv_message), 0);
			if (count < 0) {
				printf("Error in recv()\n");
			} 
			if( count == 1){
				printf("Received nothing from client\n");
			}
			if(count > 1) {
				printf("Client said: %s\n", rcv_message);

				// Parse the message that was received
				printf("\n Finding type of request");
				std::string rcv_message_string(rcv_message);

				printf("\n\n\nWe will go and fetch them what they asked for....");
				std::string responseFromServer;
				char * rcv_pointer;	// to store binary data  
				responseFromServer = getFromWebServer(rcv_message_string, &rcv_pointer);

				// Now we will check if the client made a get request 

				if(isGETRequest(rcv_message_string) == 1 && isOKAndHTMLOrText(responseFromServer) == 1 ){
					// this means the user made a get request 
					// we should check if the response sent from the web server was a 200 response, and the content 
					// is of type text or html 
					cout << "Now we should insert random errors" << endl;
					std::string errInsertedResponse = insertRandomErrors(responseFromServer);
					responseFromServer = errInsertedResponse;

					char resp[responseFromServer.size() + 1];
					strcpy(resp, responseFromServer.c_str());	// convert it to char arr
					int check_success;
					check_success = send(connected_sock, resp, sizeof(resp), 0);
					if(check_success < 0){
						printf("Error sending message back to client");
						exit(-1);
					}
					printf("Sent message back to client");

				}else{
					/// The client didn't make a get request, so just pass back the response 
					cout << "Request from client was not a GET, or not html or not text" << endl;
					cout << "We should just pass content back to client" << endl;
					cout << "rcv_pointer\n\n\n\n\n\n" << rcv_pointer << "end of rcv" << endl;

					char resp[responseFromServer.size() + 1];
					strcpy(resp, responseFromServer.c_str());	// convert it to char arr
					int check_success;
					cout << "size of rcv_pointer" << sizeof(rcv_pointer) << endl;
					cout << "resp:" << resp << "end of response" << endl;
					cout << "size of resp:" << sizeof(resp) << endl;
					check_success = send(connected_sock, rcv_pointer, 54000, 0);
					if(check_success < 0){
						printf("Error sending message back to client");
						exit(-1);
					}
					printf("Sent message back to client non get or html or text");
					}
					

			}
			if (strstr(rcv_message, "Bye") != NULL) {
				exit(0);
			}
		}



		//if (rcv_message == "Bye") {
		close(connected_sock);
		exit(0);
		//}

	}
	/* Close the socket */
	close(lstn_sock);
	return 0;
}


int isGETRequest(string getRequestString){
	printf("\n in isGETRequest");
	std::string get = "GET";
    if(get.compare(getRequestString.substr(0,3)) == 0){
        printf("\nwe have a GET\n");
        return 1;
    }
    else{
    	cout << "Not a get request, just pass response back to client" << endl;
    	return 0; 
    }
}

string getHostNameFromReq(string getRequestString){
    std::string host_str;
    std::size_t host_pos = getRequestString.find("Host:");
    if(host_pos == -1){
        printf("\nError getting host position");
        exit(0);
    }else{
        //cout << "found host position: " << host_pos << endl;
        host_str = getRequestString.substr(host_pos + 6, getRequestString.size());
        std::size_t r_position;
        r_position = host_str.find("\r");
        host_str = host_str.substr(0, r_position);
        //cout << "r_position: " << r_position << endl;
        cout << "host string:" << host_str << "::" << endl;
        return host_str;
    }
}

string getDirectoryFromReq(string getRequestString, string host_str){
	std::string directory_str; 
    std::size_t directory_pos = getRequestString.find("\n");
    directory_str = getRequestString.substr(0, directory_pos);
    cout << "first occurence of new line:" << directory_pos << endl;
    cout << "directory_str:" << directory_str << endl;
    std::size_t HTTP_sp_pos = directory_str.find("HTTP/");
    directory_str = directory_str.substr(0, HTTP_sp_pos-1);
    cout << "occurence of HTTP/:" << HTTP_sp_pos << endl;
    std::size_t space_pos = directory_str.find(" ");
    directory_str = directory_str.substr(space_pos + 1,HTTP_sp_pos-1);
    //directory_str = directory_str.substr(directory_pos, )
    cout << "directory_str:" << directory_str << ":" <<endl;
    // now let's remove the hostname if it's part of the url 
    std::size_t hst_pos = directory_str.find(host_str);
    if(hst_pos == -1){
        cout << "host name is not in pathname, we are fine" << endl;
    }else{
        cout << "host name in pathname, we need to remove it " << endl;
        // remove the host name from the directory string 
        directory_str = directory_str.substr(hst_pos + host_str.length(),directory_str.length());
        cout << "directory_str final:" << directory_str << ":" << endl;
        
    }
    return directory_str;
}

int isOKAndHTMLOrText(string responseFromServer){
  std::string response;
  std::size_t http_f_pos;
  cout << "entered isOKAndHTMLOrText" << endl;
  http_f_pos = responseFromServer.find("HTTP/");
  if (http_f_pos == -1)
    {
      cout << "error finding HTTP/" << endl;
    }
  else
    {
      cout << "HTTP/ position:" << http_f_pos << endl;
      cout << "response length:" << responseFromServer.length() << endl;
      response = responseFromServer.substr(http_f_pos + 9, responseFromServer.length());
      http_f_pos = response.find("\r");
      response = response.substr(0, http_f_pos);
      cout << "response" << endl;
      cout << "response:" << response << ":" << endl;
      
    }
    if(response.compare("200 OK") == 0){
        cout << "Message was 200 OK" << endl;
        // now we should check if content type was html or text
        http_f_pos = responseFromServer.find("Content-Type:");
        if(http_f_pos == -1 ){
            cout << "No content description available";
            return 0;
        }else{
            std::string content_type = responseFromServer.substr(http_f_pos + 14, responseFromServer.length());
            http_f_pos = content_type.find("\r\n");
            content_type = content_type.substr(0, http_f_pos);
            cout << "Content type:" << content_type << ":" << endl;
            if(content_type.find("html") != -1 || content_type.find("plain") != -1 ){
                cout << "we have either html or text file. We should return 1" << endl;
                return 1;
                
            }else{
                cout << "Not html or text. return 0" << endl;
                return 0; 
            }
        }
        
    }else{
        return 0;
    }
}

string getFromWebServer(string getRequestString, char ** myPointer){
			/* 											*/
		// Here we will create a new socket to fetch data from a web server
		struct sockaddr_in proxy_client;
		int CLIENTPORTNUM = 80; // port we want to connect to external web server
		memset(&proxy_client, 0, sizeof(proxy_client));
		proxy_client.sin_family = AF_INET; 
		proxy_client.sin_port = htons(CLIENTPORTNUM);
		proxy_client.sin_addr.s_addr = htonl(INADDR_ANY); // this will be changed

		// create a listening socket
		int client_socket;
		client_socket = socket(AF_INET, SOCK_STREAM, 0 );
		if(client_socket < 0 ){
			printf("\nError in creating client socket");
		}
		struct hostent *host_ptr;
		std::string host_name = getHostNameFromReq(getRequestString);
		std::string directory = getDirectoryFromReq(getRequestString, host_name); // MANUAL FOR NOW
		char url[host_name.size() + 1];
		strcpy(url, host_name.c_str());	// convert it to char arr
		host_ptr = gethostbyname(url);	// need to get IP address of domain
		struct in_addr **addr_list; 
		addr_list = (struct in_addr **) host_ptr->h_addr_list; 

		// copy IP address 
		for(int n = 0; addr_list[n] != NULL; n++){
			proxy_client.sin_addr = *addr_list[n];
		}

		// connect to a TCP server 
		int client_connect_check;
		client_connect_check = connect(client_socket, (struct sockaddr *) &proxy_client, sizeof(struct sockaddr_in));
		if(client_connect_check < 0){
			printf("Error connecting to web server\n");
		}
		else{
			printf("Connected.\n");
		}


		std::string get_http = "GET " + directory + " HTTP/1.1\r\nHost: " + host_name + "\r\nConnection: close\r\n\r\n";
		char cArr[get_http.size()+1];
		strcpy(cArr, get_http.c_str());
		int check_web_server_send;
		check_web_server_send = send(client_socket, cArr, sizeof(cArr), 0);
		if (check_web_server_send < 0) {
			printf("Error in send() to web server\n");
		}

		/* Receive data */
		char rcv_message[54000];
		int errorChecker;
		rcv_message[0] = '\0';				// make sure we don't receive garbage subsequent reads
		bzero(rcv_message, 54000);
		void *rcv_message2;
		errorChecker = recv(client_socket, rcv_message, sizeof(rcv_message), 0);
		if (errorChecker < 0) {
			printf("Error in recv() from web server\n");
		} else {
			printf("Server sent us this: %s\n", rcv_message);
			printf("copying content to pointer ");
			*myPointer = rcv_message;
			//printf("Pointer content: %s", myPointer); 
		}

		std::string res_str(rcv_message);


		// here we will take out the Connection: close part from what we got from that server
		std::size_t con_pos = res_str.find("Connection:");
		std::string con_manip_str = res_str;
		std::string str_before_c = con_manip_str.substr(0, con_pos);
		con_manip_str = con_manip_str.substr(con_pos);
		con_pos = con_manip_str.find("\r\n");
		std::string str_after_c = con_manip_str.substr(con_pos + 2);
		cout << "str before c:\n" << str_before_c << endl;
		cout << "str after c:\n" << str_after_c << endl;
		std::string final_string = str_before_c + str_after_c;
		cout << "Final string:\n" << final_string << endl; 

		return final_string;

}

string insertRandomErrors(string response){
	  
  //here we will find the content returned from the http browser
  cout << "\n\n\n\nResponse received from web: \n" << response << endl;
  std::size_t content_pos;
  std::string http_or_text;
  std::string responseFromServer(response);
  content_pos = responseFromServer.find("Content-Type:");

  if(content_pos == -1 ){
     cout << "error finding position of content" << endl;
     return responseFromServer;
  }else{
      // let's figure out if this is html or just simple text
      int text_pos_in_original = content_pos + 15;      // where actual content begins
      http_or_text = responseFromServer.substr(content_pos +15, responseFromServer.length());
      std::size_t content_pos = http_or_text.find("\r\n");
      if(content_pos == -1){
      	printf("Error finding r n");
      	exit(-1);
      }else{
      	printf("found r n");
      }
      http_or_text = http_or_text.substr(0, content_pos); // clips out \r\n
      text_pos_in_original = text_pos_in_original + content_pos;
      cout << "http_or_text: " << http_or_text << endl; 
      content_pos = http_or_text.find("html");
      std::size_t plain_t_pos = http_or_text.find("plain");
      std::size_t css_pos = http_or_text.find("css");  
      std::string text_content;
      if(content_pos == -1){
          printf("\ncontent type is just plaintext");
          cout << "content type is just plain text" << endl;
          //content_pos = responseFromServer.find("Content-Type:");
          text_content = responseFromServer.substr(text_pos_in_original);
          //text_pos_in_original = text_pos_in_original + content_pos;
          //content_pos = text_content.find("\r\n");
          //text_pos_in_original = text_pos_in_original + content_pos;
          text_pos_in_original = text_pos_in_original + 4; // because of special characters   
          text_content = text_content.substr( 4, text_content.length() - 4);	// removing special characters
          cout << "text_content:" << text_content << endl;
          cout << "text_content length:" << text_content.length() << endl;
          responseFromServer;

          // now we will simply insert the character X on the letter of the first two words 
          //std::string first_word;
          //content_pos = text_content.find(" ");
          //first_word = text_content.substr(0, content_pos);
          //std::string error_first_word = "Z";
          //cout << "first word:" << first_word << first_word.length() << endl;
          srand (time(NULL));
          for(int k = 1; k <=100; k++){
            int rand_int = rand() % text_content.length() - 1;
            if(rand_int <= text_content.length()){
            	text_content.replace(rand_int,1,"Z");		// at some random position, insert a Z 	
            }	 
          }
          std::string err_inserted_msg_f = responseFromServer;
          //text_content.replace(text_content.end()-4, text_content.end(), "\r\n\r\n");	// make sure file ends properly
          err_inserted_msg_f.replace(text_pos_in_original, text_pos_in_original + text_content.length(), text_content);
          err_inserted_msg_f.replace(err_inserted_msg_f.end()-4, err_inserted_msg_f.end(), "\r\n\r\n");	// make sure file ends properly

   


          //cout << "err_inserted_msg_f:\n" << err_inserted_msg_f << "\nEND" << endl;
          responseFromServer.replace(responseFromServer.end()-10, responseFromServer.end()-6, "ZZZZ");


          //cout << "printing everything character by character" << endl;
          //cout << "response from server Length:" << responseFromServer.length() << endl;
          //cout << "err_inserted_msg_f length:" << err_inserted_msg_f.length() << endl;
          //cout << "chart at 0" << responseFromServer.at(0) << endl;


          /*
          for(int index=0; index < responseFromServer.length() ; index++){
          	cout << "Char at" << index << ": " << responseFromServer.at(index) << endl;
          }
          */
          return err_inserted_msg_f;

      }else{
          cout << "content type is html" << endl;
          std::string html_content = responseFromServer.substr(text_pos_in_original, responseFromServer.length());
          //text_pos_in_original = text_pos_in_original + content_pos;
          //content_pos = text_content.find("\r\n");
          //text_pos_in_original = text_pos_in_original + content_pos;
          int html_pos_in_original = text_pos_in_original + 4; // because of special characters   
          html_content = html_content.substr( 4, html_content.length() - 4);	// removing special characters
          cout << "html_content:\n\n" << html_content << endl;
          cout << "html_content_size:" << html_content.length() << endl;


          // Now we need to find where the body content in the html file is 
          std::size_t body_bgn = html_content.find("<body>");
          std::size_t body_end = html_content.find("</body>");
          if(body_bgn != -1 && body_end != -1){
          	// find two random places to enter errors  
          	int numErrorsToInsert = 0;
          	int pos_to_look = body_bgn + 6; 
			std::string html_body = html_content.substr(pos_to_look, body_end);	// just get the body componenet of html
			std::size_t first_close_tag_pos;
			std::size_t open_tag_pos; 
			int insertAt;
			std::string err_html = html_body; 
			cout << "body:\n" << html_body << endl;
          	while( pos_to_look < body_end && numErrorsToInsert < 10){
          		first_close_tag_pos = err_html.find(">");
 				err_html = err_html.substr(first_close_tag_pos, err_html.length());
 				insertAt = pos_to_look + first_close_tag_pos;
          		open_tag_pos = err_html.find("<");
          		pos_to_look += first_close_tag_pos;
          		pos_to_look += open_tag_pos; 
          		insertAt = insertAt + 1;
          		err_html = err_html.substr(1, open_tag_pos-1);
          		if(err_html.length() > 9 && numErrorsToInsert > 0){
          		// if we have enough space to enter some bold random z, then we will 
	          		cout << "part we will enter our error:\n" << err_html << "\n End of err part" << endl;
	          		srand (time(NULL));
	            	int rand_int = rand() % (err_html.length() - 9);
	            	cout << "\nChar in rand: " << err_html.at(rand_int) << err_html.at(rand_int+1) << err_html.at(rand_int+2) << endl;
	            	cout << "Char in html body:"  << html_body.at(insertAt + rand_int) << html_body.at(insertAt + rand_int + 1) << html_body.at(insertAt + rand_int + 2) << endl; 
	            	html_body.replace(insertAt + rand_int ,8,"<b>Z</b>");		// at some random position, insert a Z 	

          		}
          		open_tag_pos = err_html.find(">");
          		pos_to_look += open_tag_pos;
          		err_html = html_body.substr(pos_to_look);
          		numErrorsToInsert += 1; 

          	}
          	//cout << "html_body:\n\n\n\n" << html_body << "\nEnd error inserted area" << endl;

          	std::string responseToSendBack = responseFromServer.replace(body_bgn + html_pos_in_original +6, html_body.length(), html_body);
          	cout << "Response with errors:\n" << responseToSendBack << endl;

	        return responseFromServer;

          }else{
          	cout << "We do not have body tags, just passing back original content" << endl;
          	return responseFromServer;
          }




          return responseFromServer;
      }


  }
}


// try bcopy on gif binary data files

