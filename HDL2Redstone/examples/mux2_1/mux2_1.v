module multiplexer2_1 ( a ,b ,x ,dout );

output dout;
input a, b, x;

assign dout = x ? b : a;
endmodule
