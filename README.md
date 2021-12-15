#Yuriy Mikhailidi

##Programing Assigment 4

##Distributed file system.

###Build APP

For building the app, you simply have to invoke "make all". It will build two targets dfs and dfc.

For clean up "make clean". You will have to manually delete any user dir. in server.
###Run APP

To run the app you first have to provide the server information and start servers via: 
####./dfs <server_name> <server_port>

To run the client you have to provide the config file for user, either from main dir. or config dir:
####./dfc <config_filepath>

Alternative you have been provided with script that will start with default config for server:

####./restartServers

Once the client starts, you can proceed with command get <filename>, put <filename> or list.

If you wish to stop runtime you can exit via CTRL^C for client. You will have to manually kill the server.

####Note: only athurized users will be allowed to preform the commands

####Resources Used For Project:
* Programming Assigment 1-3
* https://c-for-dummies.com/blog/?p=3246
* https://www.openssl.org/docs/man1.1.1/man3/MD5.html
* https://stackoverflow.com/questions/7627723/how-to-create-a-md5-hash-of-a-string-in-c
* https://stackoverflow.com/questions/10324611/how-to-calculate-the-md5-hash-of-a-large-file-in-c
* https://stackoverflow.com/questions/10147990/how-to-create-directory-with-right-permissions-using-c-on-posix
* https://stackoverflow.com/questions/7430248/creating-a-new-directory-in-c
* https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
* https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories/30602701#30602701