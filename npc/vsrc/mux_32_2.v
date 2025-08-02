module mux_32_2(
    input [31:0]data1,
    input [31:0]data2,
    input sel,
    output reg [31:0]out
);
    always@(*)begin
        case(sel)
            0:out=data1;
            1:out=data2;
        endcase
    end
endmodule

