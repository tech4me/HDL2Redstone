module adder_4 (a, b, cin, sout);
	input [3:0] a, b;
	input cin; 
	output [4:0] sout;

	wire c0, c1, c2;

	FA fa1 (a[0], b[0], cin, c0, sout[0]);
	FA fa2 (a[1], b[1], c0, c1, sout[1]);
	FA fa3 (a[2], b[2], c1, c2, sout[2]);
	FA fa4 (a[3], b[3], c2, sout[4], sout[3]);
        
        /*assign sout[0] = (a[]^cin)^b[]^c;
        assign sout[1] = (a[]^cin)^b[]^c;
        assign sout[2] = (a[]^cin)^b[]^c;*/

endmodule

module FA (a, b, cin, cout, s);
	input a, b, cin;
	output cout, s;

	assign s = a^b^cin;
	assign cout = (a&b) | (a&cin) | (b&cin);

endmodule
