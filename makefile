all:		otp_enc otp_enc_d otp_dec otp_dec_d keygen

enc:		otp_enc otp_enc_d

dec:		otp_dec otp_dec_d

keygen.o:	keygen.c
		gcc -c keygen.c -o keygen.o -Wall

encode.o:	encode.h encode.c 
		gcc -c encode.c -o encode.o -Wall

otp_enc:	encode.o otp_enc.c
		gcc -o otp_enc otp_enc.c encode.o -Wall 

otp_enc_d:	encode.o otp_enc_d.c
		gcc -o otp_enc_d otp_enc_d.c encode.o -Wall 

decode.o:	decode.h decode.c
		gcc -c decode.c -o decode.o -Wall

otp_dec:	decode.o otp_dec.c
		gcc -o otp_dec otp_dec.c decode.o -Wall

otp_dec_d:	decode.o otp_dec_d.c
		gcc -o otp_dec_d otp_dec_d.c decode.o -Wall

clean_enc:
		rm -f otp_enc otp_enc_d

clean_dec:
		rm -f otp_dec otp_dec_d
		
clean_servers:
		rm -f otp_enc_d otp_dec_d

clean_clients:
		rm -f otp_enc otp_dec

clean: 
		rm -f *.o otp_dec_d otp_dec otp_enc_d otp_enc keygen

#The debugging try everything "Special"
#-Wall -Wextra -Wformat=2 -Wswitch-default -Wcast-align -Wpointer-arith -Wbad-function-cast -Wstrict-prototypes -Winline -Wundef -Wnested-externs -Wcast-qual -Wshadow -Wwrite-strings -Wconversion -Wunreachable-code -Wstrict-aliasing=2 -ffloat-store -fno-common -lm -std=c89 -pedantic -O0 -ggdb3 -pg --coverage