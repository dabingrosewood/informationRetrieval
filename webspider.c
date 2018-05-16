#include <stdio.h>
#include <stdlib.h>
#include <sys/malloc.h>
#include <string.h>
#include <curl/curl.h>
#include "htmlstreamparser.h"
#include "BSTree.h"
#include "crc.h"
 
#define MAXQSIZE 9000000       // Maximum size of the queue, q
#define MAXURL 100000          // Maximum size of a URL
#define MAXPAGESIZE 20000          // Maximum size of a webpage
#define MAXDOWNLOADS 200      // Maximum number of downloads we will attempt
 
 FILE *tmp2;
 char *globalpointer;
 int enctypt(char *string)
{
  int sum=0;
  int i;
  for (i = 0; i < strlen(string); ++i)
  {
    sum=sum+i*string[i]/2;
  }
  return sum;
}

char *Read_file(char *filename){
  FILE * pFile;  
    long lSize;  
    char * buffer;  
    size_t result; 

  pFile = fopen (filename, "rb" );   
    if (pFile==NULL)  
    {  
        fputs ("File error",stderr);  
        exit (1);  
    }  
  
    /* 获取文件大小 */  
    fseek (pFile , 0 , SEEK_END);  
    lSize = ftell (pFile);  
    rewind (pFile);  
  
    /* 分配内存存储整个文件 */   
    buffer = (char*) malloc (sizeof(char)*lSize);  
    if (buffer == NULL)  
    {  
        fputs ("Memory error",stderr);   
        exit (2);  
    }  
  
    /* 将文件拷贝到buffer中 */  
    result = fread (buffer,1,lSize,pFile); 
    if (result != lSize)  
    {  
        fputs ("Reading error",stderr);  
        exit (3);  
    } 
    return buffer; 

}
char *GetWebPage(char *url){ 
  char *p1;
  char teststring[]="<html><body><a href=\"http://www.liacs.nl/index.html\">LIACS</a><p><a href=\"http://www.leidenuniv.nl/home.html\">Leiden</a><p><a href=\"http://www.liacs.leidenuniv.nl/index.html\">LIACS2</a></body></html>";
  p1 = ((char *) malloc(MAXPAGESIZE));
  strcpy(p1,teststring);
  return(p1);
}


void store_webpage(char *buffer,char *myurl){
  int c=mkdir("repository",0777);
  char filename[20];
  crc64(myurl,filename);
  char* location=malloc(strlen(filename)+strlen("repository/")+1);


  strcpy(location,"repository/");
  strcat(location,filename);
  // printf("filename=%s\n",location );
 
  FILE *fp; 
  fp = fopen(location,"w");
  fprintf(fp,"%s\n",buffer);
  fclose(fp);

}


void titleIndex(char *input,char *url,char *type){
//this function is used to index title of index.
    int len=strlen(input);
    FILE *fp;
    char *tmp=malloc(len);
    char *filelocation=malloc(len+strlen(type)+1);
    int d=mkdir(type,0777);
    int i;
    // printf("c=%d\n",c );
    strcpy(tmp,"");
    // char *c=malloc(1);
    char c[1];//for linux use

    // printf("title=%s\n",input );
    for (i = 0; i < len; ++i)
    {
        if ((input[i]>='A' && input[i]<='Z') || (input[i]>='a' && input[i]<='z'))
          //only consider the consecutive words
        {
            strncpy(c, input+i, 1);
            if (strlen(c)!=1)
              //sometimes c will get "http/1.1 , still not sure what cause this."
            {
              printf("c got some strange thing,c=%c\n",c);
              continue;
            }

            strcat(tmp,c);
        }else{
          if (strlen(tmp)<1)
          {
            strcpy(tmp,"");
            continue;
          }

           strcpy(filelocation,type);
           strcat(filelocation,"/");
           strcat(filelocation,tmp);

           fp = fopen(filelocation,"a");
           fprintf(fp,"%s\n",url);
           fclose(fp);

           strcpy(tmp,"");
           // reset the tmp
        }
    }
    printf("%s finished\n",type);
}


void imgindex(char *title,char *imgurl,char *type){
//this function is to index image link by the title of webpage.because of the structure is similiar with title index so i invoke it and modify some parameters.
  titleIndex(title,imgurl,type);
}


static size_t page_callback(char *buffer, size_t size, size_t nmemb, HTMLSTREAMPARSER *hsp)
{ 
  /*the size of the received data*/
  size_t realsize = size * nmemb, p; 
  char *myurl=globalpointer;

  char c, tag[6], *title, inner[128];
        size_t title_len = 0;
  char *towrite=malloc(MAXURL);
  char *titlerecord=malloc(MAXURL);
 //here to index webpage
  store_webpage(buffer,myurl);
  FILE *fp;
  //here to index webpage

        html_parser_set_tag_buffer(hsp, tag, sizeof(tag));
        html_parser_set_inner_text_buffer(hsp, inner, sizeof(inner)-1);

  for (p = 0; p < realsize; p++) 
  {             
    html_parser_char_parse(hsp, ((char *)buffer)[p]);/*Parse the char specified by the char argument*/  
    //this part to build title index.
    if (html_parser_cmp_tag(hsp, "/title", 6)) {
                        // printf("enter!\n" );
                        title_len = html_parser_inner_text_length(hsp);
                        title = html_parser_replace_spaces(html_parser_trim(html_parser_inner_text(hsp), &title_len), &title_len);
                        
                        fp = fopen("tmp","w");
                        fprintf(fp,"%s",title);
                        fclose(fp);
                        //-----here to build index----//
                        titleIndex(title,myurl,"titleindex");
                        //-----here to build index----//
                        break; // or html_parser_release_inner_text_buffer to continue
                }
    if (html_parser_cmp_tag(hsp, "a", 1)) /*Compares the tag name and the string “a”.The argument 1 is a string length.Returns 1 for equality otherwise returns 0*/   
      if (html_parser_cmp_attr(hsp, "href", 4))/*Compares the attribute name and the string “href”. The argument 4 is a string length.Returns 1 for equality otherwise returns 0*/  
        if (html_parser_is_in(hsp, HTML_VALUE_ENDED))/*Returns 1 if the parser is inside HTML_VALUE_ENDED part of HTML code.*/
        { 
          html_parser_val(hsp)[html_parser_val_length(hsp)] = '\0';
          char *towrite=html_parser_val(hsp); 
          if (strstr(towrite,myurl)==NULL && strstr(towrite,"://")==NULL) 
          {
            fwrite(myurl,strlen(myurl),1,(FILE *)tmp2);
          }
          fwrite(towrite,html_parser_val_length(hsp),1,(FILE *)tmp2);
          fwrite("\n",1,1,(FILE *)tmp2);
        }

     //this part to build img index.   
    if (html_parser_cmp_tag(hsp, "img", 3)) /*Compares the tag name and the string “a”.The argument 1 is a string length.Returns 1 for equality otherwise returns 0*/   
      if (html_parser_cmp_attr(hsp, "src", 3))/*Compares the attribute name and the string “href”. The argument 4 is a string length.Returns 1 for equality otherwise returns 0*/  
        if (html_parser_is_in(hsp, HTML_VALUE_ENDED))/*Returns 1 if the parser is inside HTML_VALUE_ENDED part of HTML code.*/
        { 
          html_parser_val(hsp)[html_parser_val_length(hsp)] = '\0';
          printf("imagelinks==%s\n",html_parser_val(hsp));
          if (strstr(html_parser_val(hsp),myurl)==NULL && strstr(html_parser_val(hsp),"//")==NULL) 
          {
            fwrite(myurl,strlen(myurl),1,(FILE *)tmp2);
            strcpy(towrite,myurl);
            strcat(towrite,html_parser_val(hsp));
          }else{strcpy(towrite,html_parser_val(hsp));}
          titlerecord=Read_file("./tmp");
          imgindex(titlerecord,towrite,"imgindex");
        } 
  }
  return realsize;
} 

char *GetLinksFromWebPage(char *myhtmlpage,char *myurl){
  tmp2=fopen("./weblinksdata.html", "w");
  /*a pointer to the HTMLSTREAMPARSER structure and initialization*/
  globalpointer=myurl;
  HTMLSTREAMPARSER *hsp = html_parser_init( ); 
  char tag[20];
  char attr[100];
  char val[128];  
  html_parser_set_tag_to_lower(hsp, 1);   
  html_parser_set_attr_to_lower(hsp, 1); 
  html_parser_set_tag_buffer(hsp, tag, sizeof(tag));  
  html_parser_set_attr_buffer(hsp, attr, sizeof(attr));    
  html_parser_set_val_buffer(hsp, val, sizeof(val)-1); 
  char *data;

  /*curl_easy_init() initializes curl and this call must have a corresponding call to curl_easy_cleanup();*/   
  CURL *curl = curl_easy_init();

  /*tell curl the URL address we are going to download*/
  curl_easy_setopt(curl, CURLOPT_URL, myhtmlpage);
  
  /*Pass a pointer to the function write_callback( char *ptr, size_t size, size_t nmemb, void *userdata); write_callback gets called by libcurl as soon as there is data received, and we can process the received data, such as saving and weblinks extraction. */
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, page_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, hsp);

  /*A parameter set to 1 tells the library to follow any Location: header that the server sends as part of a HTTP header.This means that the library will re-send the same request on the new location and follow new Location: headers all the way until no more such headers are returned.*/
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    
  /*allow curl to perform the action*/
  CURLcode curl_res = curl_easy_perform(curl);

  if(curl_res==0)
  { 
    printf("weblinks extration success\n"); 
      curl_easy_cleanup(curl);
  }
  else 
  { 
    printf("ERROR in extration weblinks\n"); 
    curl_easy_cleanup(curl); 
  } 
  /*release all the previously allocated memory ,and it corresponds to the function curl_easy_init();*/


  fclose(tmp2);
  html_parser_cleanup(hsp);

  // 读取文件方法
  data=Read_file("./weblinksdata.html");
  if(data!=NULL)
    return data;
  else
    return NULL;
}

int QSize(char *q)
{
  int k, total;
  total = 0;
  for(k=0;k<MAXQSIZE;k++)
  {
    if(q[k] == '\n') {total++;}
    if(q[k] == '\0') {return(total);}
  }
  return(total);
}

void AppendLinks(char *p, char *q, char *weblinks)
//this is not used anymore ,old version without duplicate detection
{ 
   //  *****ADD YOUR CODE HERE*****
  int q_size_now=QSize(q);
  int q_len_now=strlen(q);
  if(q_size_now+QSize(weblinks)<=MAXQSIZE)
  {
    if(q[q_len_now]=='\0')
      {
        strcat(q,weblinks);
      }
  }else
  {
    printf("Reached end of queue...exiting\n");
    exit(0);
  }
   //  You should insert the correct code here
   //  If we have reached the end of q (MAXQSIZE), then 
   //  print a short message saying "Reached end of queue...exiting"
   //  and call exit(0) to stop the program.
}





int GetNextURL(char *p, char *q, char *myurl)
//
//  This function puts the next URL from p into myurl.
//  We also pass the queue so that we can check for the end of the queue
//
{
  int k;
  for(k=0;k<MAXURL;k++)
  {
    if(p[k]=='\n')
    {
      myurl[k] = '\0';
      return(1);
    }
    else {myurl[k] = p[k];}
  }
  strcpy(myurl,"http://127.0.0.1");
  return(0);
}

char *ShiftP(char *p, char *q)
//
//  This function returns the shifted position of p to the next URL
//  We also pass the queue so that we can check for the end of the queue
//
{
  int k;
  for(k=0;k<MAXURL;k++)
  {
    if(p[k]=='\n') {p = p + k + 1;return(p);}
    if (p[k]=='\0'){printf("no more weblinks in quene!\n");p=p+k; return(p);}
  }
  return(p);
}

void linkIndex(char *url,char *weblinks){

  //this is the funciton to index weblink.
  char *point;
  char *to_add=malloc(MAXURL);
  char filename[20];
  FILE *fp; 
  char* location=malloc(20+strlen("linkindex/")+1);

  int c=mkdir("linkindex",0777);

  point=weblinks;
  int i;
  for (i = 0; i < QSize(weblinks); ++i)
  {
    if (strstr(url,"http")==NULL)
      break;
    if (GetNextURL(point,weblinks,to_add)==1)
    {
      if (strstr(to_add,"http")==NULL||strstr(to_add,"<a href>"))
      {
        printf("to_add==%s  is not a valid url,will continue...\n",to_add);
        continue;
      }
      // printf("writing to linkindex %s\n", to_add);
      crc64(to_add,filename);
      // printf("filename=%s\n", filename);

      strcpy(location,"linkindex/");
      strcat(location,filename);
      // printf("filename=%s\n", location);

      
      fp = fopen(location,"a");
      fprintf(fp,"%s\n",url);
      fclose(fp);
      
      point=ShiftP(point,weblinks);
    }
  }
  printf("Linkindex finished\n");

}

void AppendLinks2(char *q, char *weblinks,Search_tree *t)
//this function will add the weblinks into the q without duplicate
{
  char *point;
  char *to_add=malloc(MAXURL);
  char *changeline="\n";
  point=weblinks;
  int weblinksize=QSize(weblinks);
  int i;
  printf("appending links......we have %d weblinks to append \n", weblinksize);

  for (i = 0; i < weblinksize; ++i)
  {
    if (GetNextURL(point,weblinks,to_add)==1)
    {
      // printf("to_add=%s\n",to_add );
      if (strstr(to_add,"http")==NULL||strstr(to_add,"<a href>"))
      {
        printf("to_add==%s  is not a valid url,will continue...\n",to_add);
        continue;
      }
      if (!search_tree_find(t,enctypt(to_add)))
      {
        // printf("not existing in q\n");
        search_tree_insert(t,enctypt(to_add));

        if((QSize(q)+1) <= MAXQSIZE)
        {
          if(q[strlen(q)]=='\0')
            {
              strcat(q,changeline);
              strcat(q,to_add);
              // strcat(q,'\n');
            }
        }else
        {
          printf("Reached end of queue...exiting\n");
          exit(0);
        }
      }
    point=ShiftP(point,weblinks);
  }
}  
}


void weblinkIndex(char *input){
  //this is used to index weblinks.
    int len=strlen(input);
    FILE *fp;
    char *tmp=malloc(MAXURL);
    char *filelocation=malloc(MAXURL);
    int c=mkdir("webindex",0777);
    int i;
    // printf("c=%d\n",c );
    strcpy(tmp,"");

    printf("indexing weblink=%s\n",input );

    for (i = 0; i <= len; ++i)
    {
        if ((input[i]>='A' && input[i]<='Z') || (input[i]>='a' && input[i]<='z'))
          //only consider the consecutive words
        {
            // char *c=malloc(1);
            char c[1];//for linux use
            strncpy(c, input+i, 1);

            if (strlen(c)!=1)
              //sometimes c will get "http/1.1 , still not sure what cause this."
            {
              printf("c=%s\n",c );
              printf("c got some strange thing\n");
              strcpy(c,"h");
            }

            strcat(tmp,c);
        }else{
          //append the url into correspond files
          if (strlen(tmp)<1 || !strcmp(tmp,"http"))
          {
            strcpy(tmp,"");
            continue;
          }
           strcpy(filelocation,"webindex/");
           strcat(filelocation,tmp);
           // printf("%s\n",filelocation );

           fp = fopen(filelocation,"a");
           fprintf(fp,"%s\n",input);
           fclose(fp);


           strcpy(tmp,"");
           // strcpy(filelocation,"");
        }
    }
    printf("webindex finished\n");
}

int main(int argc, char* argv[]) 
{
    char *url = NULL;
    char *p, *q;
    char urlspace[MAXURL];
    char *html, *weblinks, *imagelinks;
    int k, qs, ql;
    int v;

//
// We will be putting all of the links found into q, short for "queue", and terminate with '\0'
// Initially, it will just end at MAXQSIZE, however, we could easily turn it into a ring queue
// where the end meets the beginning
//
    q = ((char *) malloc(MAXQSIZE));  // When done, use free(q) to free the memory back to the OS
    if (q==NULL) {printf("\n\nProblem with malloc...exiting\n\n"); exit(0);}
    q[0]='\0';
    p = q;

    if (argc <= 1) {printf("\n\nNo webpage given...exiting\n\n"); exit(0);} 
    else { 
      url = argv[1];
      if(strstr(url,"http") != NULL) {printf("\nInitial web URL: %s\n\n", url);}
      else {printf("\n\nYou must start the URL with lowercase http...exiting\n\n"); exit(0);}
    }
// 
//  Place the input URL into q and end it with '\n'
//

    // Search_tree *root = search_tree_create(url);
    char *a=argv[1];
    char *b="https://plus.google.com/+UniversiteitLeiden\0";

    //initialization of linked tree
    Search_tree *t;
    t=search_tree_insert(NULL,enctypt(a));

    

    strcat(q, url);  strcat(q, "\n");
    url = urlspace;
//
//  The loop limitation, MAXDOWNLOADS is the maximum number of downloads we
//  will allow the robot to perform.  It is just a precaution for this assignment
//  to minimize runaway bots
//
    for(k=0;k<MAXDOWNLOADS;k++)
    {
      qs = QSize(q); ql = strlen(q);
      printf("\nDownload #: %d   Weblinks: %d   Queue Size: %d\n",k, qs, ql);
//
//  As we go through the queue, we need to grab the next URL and shift the position of p
//  The method using the two functions GetNextURL and ShiftP was written for clarity.  
//  
      //
      //  *****ADD YOUR CODE HERE*****
      if(GetNextURL(p,q,url))
      {
        html=url; 
      }


      // Add code here to get the next URL and shift the position of q within q.
      //


//
//  This is a simple test to see if the url is within one of the allowed domains
//  liacs.nl, leidenuniv.nl, leiden.edu, or mastersinleiden.nl.  It is
//  not perfect but will probably work for most of the cases and was trivial to write.
//    
      // 
      if((strstr(url,"leidenuniv.nl") != NULL) || (strstr(url,"liacs.nl") != NULL)|| (strstr(url,"leiden.nl") != NULL) )
      {
        // html = GetWebPage(url);

        // if(html==NULL){printf("\n\nhtml is NULL\n\n");exit(0);}

        // v = strlen(html); printf("\n\nwebpage size of %s is %d\n\n",url,v);
        if(html) { 
          printf(" now extracting %s\n",html);
          weblinks = GetLinksFromWebPage(html, url);


          // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          printf("Start to append link....\n");
          //after we got the weblinks from one page,here we index links and url itself,the index of img/title etc. are finished in GetLinksFromWebPage() directly.
          //the fucntion AppendLinks2 will append new weblinks into q with duclicate detection/
          // AppendLinks(p, q, weblinks);
          AppendLinks2(q,weblinks,t);
          weblinkIndex(html);
          linkIndex(html,weblinks);
          // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

          p=ShiftP(p,q);
        }
      }
      else {
        printf("\n\nNot in allowed domains: %s\n\n",url);
        p=ShiftP(p,q);
      }
    }
    // search_tree_order(t);
    FILE *fp;
    fp = fopen("mylinks.txt","w");
    if(fp != NULL) {fprintf(fp, "%s",q);}
    free(q);

}
