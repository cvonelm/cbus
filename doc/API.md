#the CBUS reference implentation

This API description only covers the "public" API. all other
function are meant for internal use only and may be subject to change
##Central data types

###struct CBUS\_conn
This structure holds the information on a single connection

**Fields**

- char \*address | the address of the connection
- int fd | the file descriptor
- struct CBUS\_sub \*subs | the signals the client subscribed to.
- char \*path | the path of the Unix Domain Socket, which this
connection is connected to

###struct CBUS\_sub
This structure holds information on a single signal subscription of a
connection

- char \*sender\_name | which sender we want to listen for
- char \*signal\_name | which signal name we want to listen for

###struct CBUS\_msg

- uint32\_t length  | the length of the message
- uint32\_t type | the type of the message
- uint32\_t serial | the serial of the message
- char \*token | the token
- char \*from | the sender's name
- char \*to | the receiver's name
- char \*fn\_name | the signal's or sender's name
- char \*arg\_str | string describing the arguments
- struct CBUS\_arg \*args | the arguments
- char \*msg | the message as is

Refer to the protocol definition for more information on this
data structure

###struct CBUS\_arg

- int int\_value | the value, if it is an int;
- double double\_value | the value, if it is a double;
- char \*str\_value | the value if it is a string;
- int type | the type of the argument

You may only access the \*\_value which matches the data type

#The API

##Parsing messages

###struct CBUS\_msg \*cbus\_parse\_msg(char \*msg);

**Arguments**
- char \*msg | the message to be parsed

**Returns**
- a CBUS\_msg struct representing the message, or NULL on failure

*Note:* Do not free msg, it will be freed on calling cbus\_free\_msg

###void cbus\_free\_msg(struct CBUS\_msg \*msg);

Frees a message

**Arguments**
- struct CBUS\_msg \*msg | will be freed

##Helpers

###int fn\_call\_mathches(struct CBUS\_msg \*msg, char \*to, char \*fn\_name, char \*args);
Checks wether the arguments given to this function are equal to the
corresponding fields inside the "msg" structure.

Arguments may be "", when they aren't checked for.

###int fn\_return\_mathches(struct CBUS\_msg \*msg, char \*from, char \*fn\_name, char \*args);
Checks wether the arguments given to this function are equal to the
corresponding fields inside the "msg" structure.

Arguments may be "", when they aren't checked for.

###int signal\_mathches(struct CBUS\_msg \*msg, char \*from, char \*sig\_name, char \*args);
Checks wether the arguments given to this function are equal to the
corresponding fields inside the "msg" structure.

Arguments may be "", when they aren't checked for.

###void cbus\_print\_msg(struct CBUS\_msg \*msg)

Debug call to print messages

**Arguments**
- struct CBUS\_msg \*msg | the message to be printed

##Starting and stopping the connection

###struct CBUS\_conn \*cbus\_connect(char \*address, int \*err)

**Arguments**
- char \*address | the base directory of the CBUS instance
- int \*err | An pointer to an integer filled with an error code

**Returns**

A pointer to a struct CBUS\_conn representing the connection, or 
NULL on failure

**Errors**
- CBUS\_ERR\_CONNECTION | connecting to the daemon failed

###void cbus\_disconnect(struct CBUS\_conn \*conn);

Disconnects the connection "conn" and frees the corresponding structure

##Calling functions

###int cbus\_call(struct CBUS\_conn \*conn, char \*address, char \*fn\_name,char \*args, ...)

Calls a function

**Arguments**

**Returns**
The positive serial of the sent message, or a negative error code on error

**Errors**

- CBUS\_ERR\_NO\_AUTH | the application lacks the rights to call this function
- CBUS\_ERR\_PARSE | the construction of the message failed
- CBUS\_ERR\_CONNECTION | the connection had an error
- CBUS\_ERR\_DISCONNECT | we were disconnected

###struct CBUS\_msg \*cbus\_response(struct CBUS\_conn \*conn, int \*err, char \*address, char \*fn\_name, char \*args, ...);

Calls a function and waits for the return

**Arguments**
- struct CBUS\_conn \*conn | Connection on which the function call is sent
- int \*err | A pointer to an integer which is to be filled with an error code
- char \*address | The address of the call
- char \*fn\_name | The remote function to  be called
- char \*args | A string describing the arguments
- ... | the arguments

**Returns**

The matching function return or function error or NULL, in which case 
err will be set

**Errors**

- CBUS\_ERR\_NO\_AUTH | the application lacks the rights to call this function
- CBUS\_ERR\_PARSE | the construction of the message failed
- CBUS\_ERR\_CONNECTION | the connection had an error
- CBUS\_ERR\_DISCONNECT | we were disconnected


##Answering function calls

###int cbus\_answer(struct CBUS\_conn \*conn, struct CBUS\_msg \*msg, char \*args, ...)

Answers a function call

**Arguments**
- struct CBUS\_conn \*conn | Connection on which the function call is sent
- struct CBUS\_msg \*msg | The function call to be answered
- char \*args | the structure of the arguments
- ... | the arguments

**Returns**

0 if everything went well, an error code when not

**Errors**

- CBUS\_ERR\_PARSE | Construction of the answer message failed

##Subscribing to and sending signals

###int cbus\_subscribe(struct CBUS\_conn \*conn, char \*sender, char \*sig\_name)

Subscribes to a signal

**Arguments**
- struct CBUS\_conn \*conn | Connection on which the subscribe request is sent
- char \*sender | the sender we are listening for
- char \*sig\_name | the signal name we are listening for

**Returns**

0 if everything went well, an error code when not

**Errors**
- CBUS\_ERR\_NO\_AUTH | the application lacks the rights to subscribe
- CBUS\_ERR\_CONNECTION | the connection had an error
- CBUS\_ERR\_DISCONNECT | we were disconnected

###int cbus\_emit(struct CBUS\_conn \*conn, char \*sig\_name, char \*args, ...)

**Arguments**
- struct CBUS\_conn \*conn | Connection on which the signal is sent
- char \*sig\_name | the name of the signal we want to send
- char \*args | the structure of the arguments
- ... | the arguments

**Returns**

0 if everything went well, an error code when not

**Errors**
- CBUS\_ERR\_NO\_AUTH | the application lacks rights to send this signal
- CBUS\_ERR\_PARSE | couldn't construct the signal
##Functions for interacting with the daemon

###int cbus\_request\_name(struct CBUS\_conn \*conn, char \*name)

Request a (new) name

**Arguments**
- struct CBUS\_conn \*conn | Connection which we want to be (re)named
- char \*name | the name we are demanding

**Returns**

0 if everything went well, an error code when not

**Errors**
- CBUS\_ERR\_NO\_AUTH | the application lacks the rights to get a name
- CBUS\_ERR\_PARSE | the construction of the message failed
- CBUS\_ERR\_CONNECTION | the connection had an error
- CBUS\_ERR\_DISCONNECT | we were disconnected
- CBUS\_ERR\_CONFLICT | this name conflicts with another