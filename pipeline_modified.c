    /* 1. Count WRITEBACK stage is "retired" -- This I'm giving you */
    if (pipeline[WRITEBACK].instruction_address) {
        instruction_count++;
        if (debug)
            printf("DEBUG: Retired Instruction at 0x%x, Type %d, at Time %u \n",
                   pipeline[WRITEBACK].instruction_address, pipeline[WRITEBACK].itype, pipeline_cycles);
    }

    /* 2. Check for BRANCH and correct/incorrect Branch Prediction */
    if (pipeline[DECODE].itype == BRANCH) {
        int branch_taken = 0;
        
        branch_count++;//add branch count
        
        //find whether it's necessary to consider branch
        if(pipeline[FETCH].instruction_address!=0){
            //find whether branch is taken
            if(pipeline[FETCH].instruction_address!=4+pipeline[DECODE].instruction_address){
                branch_taken=1;
            }
            //determine whether prediction is right
            if(branch_taken==branch_predict_taken){
                //add correct prediction count
                correct_branch_predictions++;
            }
            else{
                //add a nop
                pipeline_cycles++;
                int s;
                for(s= MAX_STAGES - 1; s!= DECODE;s--){
                    memcpy(&pipeline[s], &pipeline[s-1], sizeof(pipeline_t));
                }
                if(pipeline[WRITEBACK].instruction_address!=0){
                    instruction_count++;
                }
                bzero(&(pipeline[DECODE]), sizeof(pipeline_t));
            }
        }
    }

    /* 3. Check for LW delays due to use in ALU stage and if data hit/miss
     *    add delay cycles if needed.
     */
    if (pipeline[MEM].itype == LW) {
        int inserted_nop = 0;
        //determine whether the data hits
        data_hit=iplc_sim_trap_address(pipeline[MEM].stage.lw.data_address);
        if(data_hit){
            //print hit
            printf("DATA HIT:\t Address 0x%x \n", pipeline[MEM].stage.lw.data_address);
        }
        else{
            //add cycle count and print miss
            pipeline_cycles=(pipeline_cycles+CACHE_MISS_DELAY-1);
            printf("DATA MISS:\t Address 0x%x \n", pipeline[MEM].stage.lw.data_address);
        }
        
        //if ALU is r-type and needs result from previous load,add nop
        if(pipeline[ALU].itype==RTYPE)
        {
            // whether result from previous load
            int needpre=0;
            int inslength=strlen(pipeline[ALU].stage.rtype.instruction);
            if (((pipeline[ALU].stage.rtype.reg2_or_constant == pipeline[MEM].stage.lw.dest_reg)
                 && pipeline[ALU].stage.rtype.instruction[inslength-1] != 'i')||(pipeline[ALU].stage.rtype.reg1 == pipeline[MEM].stage.lw.dest_reg)){
                    needpre=1;
            }
            if(needpre)
            {
                //add nop
                if(data_hit)pipeline_cycles++;
                inserted_nop = 1;
                memcpy(&pipeline[WRITEBACK], &pipeline[MEM], sizeof(pipeline_t));
                bzero( &(pipeline[MEM]), sizeof(pipeline_t));
                
                //add instruction count if a instruction WB is passed
                if (pipeline[WRITEBACK].instruction_address)instruction_count++;
            }
        }
    }

    /* 4. Check for SW mem acess and data miss .. add delay cycles if needed */
    if (pipeline[MEM].itype == SW) {
        //determine whether the data hits
        data_hit=iplc_sim_trap_address(pipeline[MEM].stage.sw.data_address);
        if(data_hit){
            //print hit
            printf("DATA HIT:\t Address 0x%x \n",pipeline[MEM].stage.sw.data_address);
        }
        else{
            //add cycle count and print miss
            pipeline_cycles=(pipeline_cycles+CACHE_MISS_DELAY-1);
            printf("DATA MISS:\t Address 0x%x \n",pipeline[MEM].stage.sw.data_address);
        }
    }

    /* 5. Increment pipe_cycles 1 cycle for normal processing */
    pipeline_cycles++;
    /* 6. push stages thru MEM->WB, ALU->MEM, DECODE->ALU, FETCH->ALU */
    int st;
    for(st= MAX_STAGES-1;st!= FETCH;st--){
        //copy stages
        memcpy(&pipeline[st], &pipeline[st-1], sizeof(pipeline_t));
    }
    // 7. This is a give'me -- Reset the FETCH stage to NOP via bezero */
    bzero(&(pipeline[FETCH]), sizeof(pipeline_t));
}

/*
 * This function is fully implemented.  You should use this as a reference
 * for implementing the remaining instruction types.
 */
void iplc_sim_process_pipeline_rtype(char *instruction, int dest_reg, int reg1, int reg2_or_constant)
{
    /* This is an example of what you need to do for the rest */
    iplc_sim_push_pipeline_stage();

    pipeline[FETCH].itype = RTYPE;
    pipeline[FETCH].instruction_address = instruction_address;

    strcpy(pipeline[FETCH].stage.rtype.instruction, instruction);
    pipeline[FETCH].stage.rtype.reg1 = reg1;
    pipeline[FETCH].stage.rtype.reg2_or_constant = reg2_or_constant;
    pipeline[FETCH].stage.rtype.dest_reg = dest_reg;
}

void iplc_sim_process_pipeline_lw(int dest_reg, int base_reg, unsigned int data_address)
{
    /* You must implement this function */
    iplc_sim_push_pipeline_stage();
    
    //implement itype and instruction address
    pipeline[FETCH].itype = LW;
    pipeline[FETCH].instruction_address = instruction_address;
    
    //implement stage
    pipeline[FETCH].stage.lw.base_reg = base_reg;
    pipeline[FETCH].stage.lw.dest_reg = dest_reg;
    pipeline[FETCH].stage.lw.data_address = data_address;
}

void iplc_sim_process_pipeline_sw(int src_reg, int base_reg, unsigned int data_address)
{
    /* You must implement this function */
    iplc_sim_push_pipeline_stage();
    
    //implement itype and instruction address
    pipeline[FETCH].itype = SW;
    pipeline[FETCH].instruction_address = instruction_address;
    
    //implement stage
    pipeline[FETCH].stage.sw.base_reg = base_reg;
    pipeline[FETCH].stage.sw.src_reg = src_reg;
    pipeline[FETCH].stage.sw.data_address = data_address;
}

void iplc_sim_process_pipeline_branch(int reg1, int reg2)
{
    
    /* You must implement this function */
    iplc_sim_push_pipeline_stage();
    
    //implement itype and instruction address
    pipeline[FETCH].itype = BRANCH;
    pipeline[FETCH].instruction_address = instruction_address;
    
    //implement stage
    pipeline[FETCH].stage.branch.reg1 = reg1;
    pipeline[FETCH].stage.branch.reg2 = reg2;
}

void iplc_sim_process_pipeline_jump(char *instruction)
{
    /* You must implement this function */
    iplc_sim_push_pipeline_stage();
    
    //implement itype and instruction address
    pipeline[FETCH].itype = JUMP;
    pipeline[FETCH].instruction_address = instruction_address;
    
    //implement stage
    strcpy(pipeline[FETCH].stage.jump.instruction,instruction);
}

void iplc_sim_process_pipeline_syscall()
{
    /* You must implement this function */
    iplc_sim_push_pipeline_stage();
    
    //implement itype and instruction address
    pipeline[FETCH].itype = SYSCALL;
    pipeline[FETCH].instruction_address = instruction_address;
    
    //do not need implement stage
}

void iplc_sim_process_pipeline_nop()
{
    /* You must implement this function */
    iplc_sim_push_pipeline_stage();
    
    //implement itype and instruction address
    pipeline[FETCH].itype = NOP;
    pipeline[FETCH].instruction_address = instruction_address;
    
    //do not need implement stage
}