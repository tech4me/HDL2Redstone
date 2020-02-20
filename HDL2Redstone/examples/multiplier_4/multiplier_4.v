module multiplier_4(a, b, result);
	input [3:0] a, b;	
	output [7:0] result;
	
	wire [3:0] pp1, pp2, pp3, pp0;
        assign pp0 = a & {4{b[0]}}; 	
        assign pp1 = a & {4{b[1]}}; 	
        assign pp2 = a & {4{b[2]}}; 	
        assign pp3 = a & {4{b[3]}};

	wire cout1, cout2, cout3;
	wire [3:0] s1, s2, s3;

	adder_4 adder1(pp1, {1'b0, pp0[3:1]}, 1'b0, cout1, s1);	
	adder_4 adder2(pp2, {cout1, s1[3:1]}, 1'b0, cout2, s2);	
	adder_4 adder3(pp3, {cout2, s2[3:1]}, 1'b0, cout3, s3);

	assign result = {cout3, s3, s2[0], s1[0], pp0[0]};
endmodule


module adder_4 (a, b, cin, cout, sout);
	input [3:0] a, b;
	input cin; 
	output [3:0] sout;
	output cout;

	wire c0, c1, c2;

	FA fa1 (a[0], b[0], cin, c0, sout[0]);
	FA fa2 (a[1], b[1], c0, c1, sout[1]);
	FA fa3 (a[2], b[2], c1, c2, sout[2]);
	FA fa4 (a[3], b[3], c2, cout, sout[3]);
        
endmodule

module FA (a, b, cin, cout, s);
	input a, b, cin;
	output cout, s;

	assign s = a^b^cin;
	assign cout = (a&b) | (a&cin) | (b&cin);

endmodule
