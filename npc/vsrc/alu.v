module alu(
    input [31:0]pc,
    input [31:0]src1,
    input [31:0]src2,
    input [31:0]decode,
    output reg[31:0]data_out1,
    output reg[31:0]data_out2,
    output reg[4:0]waddr,
    output reg[2:0]fetch,
    output reg gpr_wen,
    output reg[31:0]dnpc,
    output reg is_ebreak
);
    /* verilator lint_off WIDTHTRUNC */
    import "DPI-C" function byte unsigned pmem_read_ram(input int raddr);
    import "DPI-C" function void pmem_write(
    input int waddr, input int wdata, input int wmask);
    reg [11:0]imm_i;
    reg [19:0]imm_u;
    reg [11:0]imm_s;
    reg [31:0]bug;
    always@(*)begin
        dnpc = pc+4;
        waddr = decode[11:7];
        casez(decode)
            32'bzzzzzzzzzzzzzzzzz000zzzzz0010011:begin
                fetch = 3'b000;
                imm_i[11:0] = decode[31:20];
                data_out1 = src1+{{20{imm_i[11]}},imm_i};
            end
            32'bzzzzzzzzzzzzzzzzz000zzzzz1100111:begin
                fetch = 3'b001;
                imm_i[11:0] = decode[31:20];
                data_out2 = src1+{{20{imm_i[11]}},imm_i};
                data_out1 = pc+4;
            end
            32'b0000000zzzzzzzzzz000zzzzz0110011:begin
                fetch = 3'b010;
                data_out1 = src1+src2;
            end
            32'bzzzzzzzzzzzzzzzzzzzzzzzzz0110111:begin
                fetch = 3'b011;
                imm_u[19:0] = decode[31:12];
                data_out1 = imm_u<<12;
            end
            32'bzzzzzzzzzzzzzzzzz010zzzzz0000011:begin
                fetch = 3'b100;
                imm_i[11:0] = decode[31:20];
                data_out1 = {pmem_read_ram(src1+{{20{imm_i[11]}},imm_i}+32'd3),
                             pmem_read_ram(src1+{{20{imm_i[11]}},imm_i}+32'd2),
                             pmem_read_ram(src1+{{20{imm_i[11]}},imm_i}+32'd1),
                             pmem_read_ram(src1+{{20{imm_i[11]}},imm_i})};
                
            end
            32'bzzzzzzzzzzzzzzzzz100zzzzz0000011:begin
                fetch = 3'b101;
                imm_i[11:0] = decode[31:20];
                data_out1 = {{24{1'b0}},pmem_read_ram(src1+{{20{imm_i[11]}},imm_i})};
            end
            32'bzzzzzzzzzzzzzzzzz010zzzzz0100011:begin
                fetch = 3'b110;
                imm_s[11:0] = {decode[31:25],decode[11:7]};
                pmem_write(src1+{{20{imm_s[11]}},imm_s},src2,{{28{1'b0}},4'b1111});
            end
            32'bzzzzzzzzzzzzzzzzz000zzzzz0100011:begin
                fetch = 3'b111;
                imm_s[11:0] = {decode[31:25],decode[11:7]};
                pmem_write(src1+{{20{imm_s[11]}},imm_s},src2,{{28{1'b0}},4'b0001});
            end
            32'b00000000000100000000000001110011:is_ebreak = 1;
        endcase
        if(fetch == 3'b000||fetch == 3'b001
         ||fetch == 3'b010||fetch == 3'b011
         ||fetch == 3'b100||fetch == 3'b101) begin   
            gpr_wen = 1;
        end
        else begin
            gpr_wen = 0;
        end
    end
endmodule