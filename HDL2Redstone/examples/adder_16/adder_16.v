module adder_4 (a, b, cin, sout);
	input [15:0] a, b;
	input cin; 
	output [16:0] sout;

	assign sout = a + b + cin;

endmodule
