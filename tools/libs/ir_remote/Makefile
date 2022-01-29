CC = gcc -w -O1

all:
		$(CC) raspi_ir_in.c  -o raspi_ir_in
		$(CC) -lwiringPi raspi_ir_out.c  -o raspi_ir_out

clean:
	rm -f raspi_ir_in raspi_ir_out
