test:
	cc  -o cbus_test cbus_test.c  -g3 -lcbus
	./cbus_test
call:
	cc -o cbus_call cbus_call.c -g3 -lcbus
	./cbus_call ${HOME}/.cbus
response:
	cc -o cbus_response cbus_response.c -g3 -lcbus
	./cbus_response ${HOME}/.cbus
 
emit:
	cc -o cbus_emit cbus_emit.c  -g3 -lcbus
	./cbus_emit ${HOME}/.cbus

sub:
	cc -o cbus_sub cbus_sub.c -g3 -lcbus
	./cbus_sub ${HOME}/.cbus
clean:
	rm -r cbus_sub cbus_call cbus_test cbus_emit cbus_response
.PHONY: clean
  
