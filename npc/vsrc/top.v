module top(
    input clk,
    input rst
);
    /* verilator lint_off WIDTHTRUNC */
    import "DPI-C" function byte signed pmem_read_ram(input int raddr);
    import "DPI-C" function int pmem_read_rom(input int raddr);
    import "DPI-C" function void pmem_write(
    input int waddr, input int wdata, input int wmask);
    export "DPI-C" function trap;
    function int trap();
        return is_ebreak?1:0;
    endfunction
    wire [31:0]pc_data_in;
    wire [31:0]pc_data_out;
    wire [31:0]decode;
    wire [31:0]snpc;
    wire [31:0]dnpc;
    wire [4:0]raddr1;
    wire [4:0]raddr2;
    wire [31:0]src1;
    wire [31:0]src2;
    wire [4:0]waddr;
    wire [31:0]wdata;
    wire [2:0]fetch;
    wire is_jalr;
    wire gpr_wen;
    wire is_ebreak;
    assign is_jalr = (fetch==3'b001)?1:0;
    assign decode = pmem_read_rom(pc_data_out);
    assign raddr1 = decode[19:15];
    assign raddr2 = decode[24:20];
    mux_32_2 pc_mux(
        .data1(snpc),
        .data2(dnpc),
        .sel(is_jalr),
        .out(pc_data_in)
    );
    register pc(
        .clk(clk),
        .rst(rst),
        .en(1),
        .q(pc_data_out),
        .p(pc_data_in)
    );
    alu decoder(
        .decode(decode),
        .pc(pc_data_out),
        .src1(src1),
        .src2(src2),
        .data_out1(wdata),
        .data_out2(dnpc),
        .waddr(waddr),
        .dnpc(snpc),
        .fetch(fetch),
        .is_ebreak(is_ebreak),
        .gpr_wen(gpr_wen)
    );
    RegisterFile gpr(
        .rst(rst),
        .clk(clk),
        .wen(gpr_wen),
        .ren(1),
        .wdata(wdata),
        .waddr(waddr),
        .raddr1(raddr1),
        .raddr2(raddr2),
        .rdata1(src1),
        .rdata2(src2)
    );
endmodule

    