#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <stdint.h>
 FILE *mem = NULL;
    char **mem_instr = NULL;

//struct para busca
typedef struct instrucao {
    int opcode;
    int rs;
    int rt;
    int rd;
    int funct;
    int8_t  imm;
    int  addr;
    int64_t dado;
} instrucao;
//struct para decodificação
typedef struct unidade_controle {
    int ALUSrc;
    int MemRead;
    int jump;
    //novos sinais
    int origPC;
    int ula_fonteA;
    int ula_fonteB;
    int RegWrite;
    int RegDst;
    int MemToReg;
    int Irwrite;
    int MemWrite;
    int PCwrite;
    int IouD;
    int ALUOp;
    int Branch;
}controle;

typedef struct metricas {
    int contInst;
    int contInstReg;
    int contInstImm;
    int contInstJump;
} metricas;


int registradores[8]={0, 100, -25, 0, -30, 0, 0, -90};
int memoria[256] = {0};
int oldreg[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int oldmem[256] = {0};
int oldpc=0;
char memu[256][17];

instrucao decodificar(char *bin);
void imprimir_ass (char*bin, char memu[256][17], int k);
void carregamem (char memu[256][17]);
void imprimir_mem_instr(char memu[256][17], int m, int n, char* bin);
void imprimir_reg();
void imprimir_instrucao(instrucao p);
char **criameminstr(int m, int n);
void desalocameminstr(char **mem_instr, int m, int n);
int mux1(controle c, instrucao i);
int mux_branch(int sinal_branch,int entrada1,int entrada2);
int mux_jump(int sinal_jump,int entrada1,int entrada2);
int somador(int entrada1,int entrada2);
instrucao busca (char *bin, char memu[256][17], int pc);
controle sinais_controle(instrucao i, metricas *m);
void executar(instrucao i, controle c, int *pc);
int ula(int op1, int op2, controle c, int *overflow,int *zero);//adicionei o zero na função da ula que vai ser utilizado para o beq
int lwsw(int operacao, int endereco, int dado);
int sign_extend6to8(int imm);
void imprimir_mem_dados(int mem[]);
void gerar_asm(instrucao p,int pc,char bin[]);
void gerar_dat(int mem[]);
void mostrar_metricas(metricas);
void carregadat (int *mem_dados);
void conversao(char bin[], int numero);
void complemento2(char bin[]);
void imprimir_memoria(char memu[256][17], int m, int n, char* bin);
int mux_IouD(controle c,int entrada1,int entrada2);
int mux_memtoreg(controle c,int entrada1,int entrada2);
int mux_RegDst(controle c,int entrada1,int entrada2);
int mux_operandoA_ULA(controle c,int entrada1,int entrada2);
int mux_operandoB_ULA(controle c,int entrada1,int entrada2,int entrada3);
int mux_fontePC(controle c,int entrada1,int entrada2,int entrada3);
controle sinais_controle_multiclo(int estado_atual);
void etapa_busca_multiciclo(int *estado_atual,char menu[256][17],int *pc,char RegIR[17]);
void etapa_decodificacao_multiciclo(int *estado_atual,char RegIR[17],int *Reg_aluout,int banco_reg[],int *reg_tempA,int *reg_tempB,int pc);
void etapa_execucao_tipoR(int *estado_atual,int reg_tempA,int reg_tempB,int *Reg_aluout);
void etapa_termino_tipoR(int *estado_atual,int Reg_aluout,char RegIR[17],int banco_reg[]);

int main() {
    FILE *mem = NULL;
    char **mem_instr = NULL;
    int m = 256;
    int n = 17;
    int escolha=1,pc=0,estado_atual=0,etapa=0;
    char bin[17];
    int Reg_tempA=0;
    int Reg_tempB=0;
    int Reg_dados=0;
    char RegIR[17];
    int Reg_aluOUT=0;
    instrucao i;
    controle c;
    metricas metricas = {0};
    mem_instr = criameminstr(m, n);
    int temp_pc=0;
    
    printf("\n\nMenu de opcoes do programa");
    do { printf("\n\n[1] Carregar memoria de instrucao");
     printf("\n[2] executar um stap no multiclo");
     printf("\n[3] Imprimir memoria de instrucoes e dados");
     printf("\n[4] Imprimir banco de registradores");
     printf("\n[5] Imprimir todo simulador");
     printf("\n[6] Salvar .asm e .dat");
     printf("\n[7] Mostrar Estatisticas do programa");
     printf("\n[8] Executar programa(RUN)");
     printf("\n[9] Executar uma instruçao(STEP)");
     printf("\n[10] Voltar uma instrucao");
     printf("\n[0] Encerrar programa");
     printf("\nescolha uma opcao: ");
     scanf("%d",&escolha);
     switch (escolha) {
         case 1:
             printf("\nCarregando memoria\n");
             carregamem(memu);
             break;
         case 2: 
            //O GURIS EU COLOQUEI ISSO PORQUE COMEÇAR ALI NO CASE 9 EU ACHEI MEIO COMPLICADO PORQUE é MUITA COISA
            //ENTAO VOU FAZER PRIMEIRO AQUI
            switch (estado_atual)
            {
            case 0:
                etapa_busca_multiciclo(&estado_atual,memu,&pc,RegIR);
                printf("\nEstado atual:%d",estado_atual);
                printf("\nAtual valor pc:%d",pc);
                printf("\nproximo estado:%d",estado_atual);
                break;
            case 1:
                printf("\nEtapa de decodificação");
                etapa_decodificacao_multiciclo(&estado_atual,RegIR,&Reg_aluOUT,registradores,&Reg_tempA,&Reg_tempB,pc);
                printf("\nproximo estado:%d",estado_atual);
                break;

            case 7:
                printf("\nEtapa de execução de uma instrução do tipo R!");
                etapa_execucao_tipoR(&estado_atual,Reg_tempA,Reg_tempB,&Reg_aluOUT);
                printf("\nvalor do registrador saida ula:%d",Reg_aluOUT);
                printf("\nproximo estado:%d",estado_atual);
                break;
            case 8:
                printf("\nEtapa final do tipo R ");
                etapa_termino_tipoR(&estado_atual,Reg_aluOUT,RegIR,registradores);
                printf("\nproximo estado:%d",estado_atual);
            
            default:
                break;
            }
         break;
         case 3:
            imprimir_memoria(memu,m,n,bin);
         break;
         case 4: printf("\nbanco de registradores\n");
         imprimir_reg();
         break;
         case 5: printf("\nImprimindo banco de registradores e memória de dados:");
            imprimir_mem_dados(memoria);
            imprimir_reg();
            printf("Instrução executada em ");
            imprimir_instrucao(i);
            printf("\nPC da proxima instrucao:%d",pc);
         break;
         case 6:
            printf("\nArquivo  Assembly sendo gerado...");
            strcpy(bin, mem_instr[temp_pc]);
            while (strcmp(bin,"0000000000000000") !=0)
            {
                instrucao p=decodificar(bin);
                gerar_asm(p,temp_pc,bin);
                temp_pc++;
                strcpy(bin, mem_instr[temp_pc]);                
            }
            printf("\nArquivo gerado!");
            printf("\nArquivo sendo de dados sendo gerado....");
            gerar_dat(memoria);
            printf("\nArquivo gerado!");

         break;
         case 7:
            printf("Estatisticas do programa: ");
            mostrar_metricas(metricas);
         break;
         case 8:
          do{i = busca(bin, memu, pc);
           c = sinais_controle(i, &metricas);
          int old = pc;
          executar(i, c, &pc);
        if(pc == old){
          pc++;
        }
        }while(pc<=255);
        printf("Programa Executado!\n");
         break;
         case 9:
            for(int j=0;j<256;j++){
            oldmem[j] = memoria[j];}
            for(int k=0;k<8;k++){
            oldreg[k] = registradores[k];}
            oldpc = pc;
           i = busca(bin, memu, pc);
           c = sinais_controle(i, &metricas);
          executar(i, c, &pc);
          printf("Instrução executada!\n");
          printf("PC da proxima instrucao:%d\n",pc);
         break;
         case 10:
            pc = oldpc;
             for(int j=0;j<256;j++){
            memoria[j] = oldmem[j];}
            for(int k=0;k<8;k++){
            registradores[k] = oldreg[k];}
            i = busca(bin, memu, pc);
           printf("\nPC da proxima instrucao:%d",pc);
           break;
         default:
             return 0;
             break;
    }
} while (escolha !=0);
desalocameminstr(mem_instr, m, n);
return 0;
}


char **criameminstr(int m, int n){
    int i;
    char **mem_instr = NULL;
    mem_instr = (char**) malloc(m*sizeof(char*));
    for(i=0;i<m;i++){
        mem_instr[i] = (char*) malloc((n+1)*sizeof(char));
        for (int j = 0; j < 16; j++){
            mem_instr[i][j] = '0';
        }
        mem_instr[i][n] = '\0';
    } return mem_instr;
}

void desalocameminstr(char **mem_instr, int m, int n){
    int i;
    for(i=0;i<m;i++){
        free(mem_instr[i]);
    }
    free(mem_instr);
    return;
}
#include <stdio.h>
#include <string.h>

FILE *mem;

void carregamem(char memu[256][17]) {

    char arq[256];

    printf("Digite o nome do arquivo a ser lido: ");
    scanf("%s", arq);

    mem = fopen(arq, "r");
    if (mem == NULL) {
        printf("Erro ao abrir o arquivo!\n");
        return;
    }

    int addr_inst = 0;
    int addr_data = 127;
    int modo = 0;

    char aux[32];

    while (fgets(aux, sizeof(aux), mem) != NULL) {

        aux[strcspn(aux, "\n")] = '\0';

        // muda para seção de dados
        if (strcmp(aux, ".data") == 0) {
            modo = 1;
            continue;
        }

        if (strlen(aux) == 0) continue;

        if (modo == 0) {
            // instruções (0–127)
            if (addr_inst < 128) {
                strcpy(memu[addr_inst], aux);
                addr_inst++;
            }
        } else {
            // dados (128–255)
            if (addr_data < 256) {
                strcpy(memu[addr_data], aux);
                addr_data++;
            }
        }
    }

    fclose(mem);

    printf("Memoria carregada!\n");
}
void carregadat(int *mem_dados){
  char arq[256];
  setbuf(stdin, NULL);
  printf("Digite o nome do arquivo a ser lido: ");
  scanf("%s", &arq);
    mem = fopen(arq, "r");
    if (mem == NULL){
        printf("Erro ao abrir o arquivo!\n");
        return;
    }
    char c[6];
    int i = 0;
    while (i < 256 && fscanf(mem, "%5s", c) == 1){
        mem_dados[i] = atoi(c);
        i++;
    }
    fclose(mem);
    printf("\nMemória de dados carregada");
}

instrucao decodificar(char *bin) {
    unsigned int valor = strtoul(bin, NULL, 2);
    instrucao i;
    i.opcode = (valor >> 12) & 0xF;
    if (i.opcode==0) {
        //Tipo R
    i.rs = (valor >> 9) & 0x7;
    i.rt = (valor >> 6) & 0x7;
    i.rd = (valor >> 3) & 0x7;
    i.funct = valor & 0x7; }
    else if (i.opcode==2) {
        //TIPO J
        i.addr=valor & 0xff;
        i.rs=0;
        i.rd=0;
        i.rt=0;
        i.imm=0;
        i.funct=0;
        }
        else {
            //TIPO I
            i.rs=(valor >> 9) & 0x7;
            i.rt=(valor >> 6) & 0x7;
            i.imm=valor & 0x3f;
        } return i;
}

void imprimir_mem_instr(char memu[256][17], int m, int n, char* bin) {
    int k=0,j=0;
    printf("\n=======MEMORIA DE INSTRUCAO======\n");
    for ( k = 0; k < m; k++) {
      printf("Instrução %d: ", k+1);
        for ( j = 0; j < n; j++) {
            printf("%c",memu[k][j]);
        }
        imprimir_ass(bin,memu, k);
        printf("\n");
    }
}
void imprimir_ass (char*bin, char memu[256][17], int k){
    strcpy(bin, memu[k]);
    instrucao i = decodificar(bin);
    imprimir_instrucao(i); 
}

void imprimir_reg() {
    int i;
    printf("\n=====BANCO DE REGISTRADORES=====\n");
    for ( i = 0; i < 8; i++) {
        printf("[R%d] = %d ",i, registradores[i]);
    }
    printf("\n\n");
}

void imprimir_instrucao(instrucao p) {
  switch (p.opcode)
    {
    case 0:
        switch (p.funct)
        {
        case 0:
            printf("|Assembly: add $%d,$%d,$%d\n",p.rd,p.rs,p.rt);
            break;

        case 2:
            printf("|Assembly: sub $%d,$%d,$%d\n",p.rd,p.rs,p.rt);
            break;
        }
        break;

    case 2:
        printf("|Assembly: jump %d\n",p.addr);
        break;

    case 4:
        p.imm=sign_extend6to8(p.imm);
        printf("|Assembly: addi $%d,$%d,%d\n",p.rt,p.rs,p.imm);
        break;

    case 8:
        p.imm=sign_extend6to8(p.imm);
        printf("|Assembly: beq $%d,$%d,%d\n",p.rs,p.rt,p.imm);
        break;

    case 11:
        p.imm=sign_extend6to8(p.imm);
        printf("|Assembly: lw $%d,%d($%d)\n",p.rt,p.imm,p.rs);
        break;

    case 15:
        p.imm=sign_extend6to8(p.imm);
        printf("|Assembly: sw $%d,%d($%d)\n",p.rt,p.imm,p.rs);
        break;

    default:
        printf("| instrucao desconhecida\n");
        break;
    }
}

//Função que recebe a instrução convertida e decodifica os sinais de controle
controle sinais_controle(instrucao i, metricas *m){
    controle c;
    // Inicializa tudo com 0
    c.RegDst = 0;
    c.ALUSrc = 0;
    c.MemToReg = 0;
    c.RegWrite = 0;
    c.MemRead = 0;
    c.MemWrite = 0;
    c.Branch = 0;
    c.ALUOp = 0;
    c.jump = 0;
    m->contInst ++;
    switch(i.opcode){
        case 0:
            // Tipo R
            c.RegDst = 1;
            c.ALUSrc = 0;
            c.MemToReg = 0;
            c.RegWrite = 1;
            c.ALUOp = i.funct; // usa funct direto
            m->contInstReg ++;
            break;
        case 4:
            // ADDI
            c.RegDst = 0;
            c.ALUSrc = 1;
            c.RegWrite = 1;
            c.ALUOp = 0;
            m->contInstImm ++;
            break;
        case 11:
            // LW
            c.ALUSrc = 1;
            c.MemToReg = 1;
            c.RegWrite = 1;
            c.MemRead = 1;
            m->contInstImm ++;
            break;
        case 15:
            // SW
            c.ALUSrc = 1;
            c.MemWrite = 1;
            m->contInstImm ++;
            break;
        case 8:
            // BEQ
            c.Branch = 1;
            c.ALUOp = 2;
            m->contInstImm ++;
            break;
        case 2:
            // JUMP
            c.jump = 1;
            m->contInstJump ++;
            break;

    } return c;

}

//função que realiza a busca da instrução
instrucao busca (char *bin, char memu[256][17], int pc){
    strcpy(bin, memu[pc]);
    instrucao i = decodificar(bin);
    printf("\ninstrucao em binario:%s",bin);
    imprimir_instrucao(i); return i;
}

int mux1(controle c, instrucao i){
    if(c.RegDst == 0){
        return i.rt; }
    else if(c.RegDst == 1){
        return i.rd;
    } return 0;
}

int ula(int op1, int op2, controle c, int *overflow,int *zero){
    *overflow = 0;
    *zero=0;
    int res=0;
    switch(c.ALUOp){
        case 0: { 
            // ADD
            res=op1 + op2;
            //eu arrumei aqui porque o overflow no caso de 8 bits tem a faixa de -128 ate 127
            if (res>127 || res<-128){
                *overflow=1;
            }
            if (res==0){
                *zero=1;
            }
            return res;
        }
        case 2: 
         // SUB
            res=op1-op2;
            if (res>127 || res<-128){
                *overflow=1;
            }
            if (res==0){
                *zero=1;
            }
            return res;
        case 4:
            return op1 & op2;
        case 5:
            return op1 | op2;
        default:
            return 0;
    }
}

void executar(instrucao i, controle c, int *pc){
    int op1;
    int op2;
    int resultado;
    int destino;
    int saida_muxBranch=0;
    int resultado_soma_branch=0;
    int overflow;
    int zero;
    //Extensão de sinal:
    op1=registradores[i.rs];
    i.imm=sign_extend6to8(i.imm);
    // MUX da segunda entrada da ULA (ALUSrc)
    if(c.ALUSrc == 1){
        op2 = i.imm;
    } else {
        op2 = registradores[i.rt];
    }
    // Executa na ULA
    resultado = ula(op1, op2, c, &overflow,&zero);
    if(overflow){
    printf("Overflow!\n");
    return ;
  }
    // MUX do registrador destino
    destino = mux1(c, i);
    // Escreve no banco de registradores
    if(c.RegWrite){
        registradores[destino] = resultado;
    }
    // Branch
    //primeiro a gente soma o valor do imediato com o valor do pc + 1 para saber valor da entrada 1 do mux branch
    resultado_soma_branch=somador(i.imm,*pc+1);
    //agora a gente vai selecionar atraves de um mux qual vai ser caminho selecionado o da soma do branch ou do pc
    saida_muxBranch=mux_branch(c.Branch & zero,*pc+1,resultado_soma_branch);
    //agora temos o resultado selecionado pelo mux do branch
    //agora iremos fazer o mesmo com jump somar com pc +1
    //agora iremos selecionar o resultado do mux do branch com o resultado da soma do jump e o item selecionado vai virar o pc
    *pc=mux_jump(c.jump,saida_muxBranch,i.addr);
    // Sw
    if(c.MemWrite) {
        lwsw(1, resultado, i.rt);
    }
    // Lw
    if(c.MemRead) {
        registradores[destino] = lwsw(2, resultado, i.rt);
    }
}

int lwsw(int op, int endereco, int dado) {
    if (op == 1)
    {
        memoria[endereco] = registradores[dado];
        return 0;
    }
    else if (op == 2) {
        return memoria[endereco];
    }
}
int sign_extend6to8(int imm)
{
    if (imm & 0x20)      // verifica bit de sinal (bit 5)
        return imm | 0xC0;  // adiciona 11 nos 2 bits superiores
    else
        return imm;
}
int somador(int entrada1,int entrada2)
{
    int resultado=0;
    resultado=entrada1+entrada2;
    return resultado;
}
int mux_branch(int sinal_branch,int entrada1,int entrada2)
{
    switch (sinal_branch)
    {
    case 0:
        return entrada1;
        break;
    case 1: 
        return entrada2;
    default:
        break;
    }
    return 0;
}
int mux_jump(int sinal_jump,int entrada1,int entrada2)
{
    switch (sinal_jump)
    {
    case 0:
        return entrada1;
        break;
    case 1:
        return entrada2;
        break;
    default:
        break;
    }
    return 0;
}
void imprimir_mem_dados(int mem[]){
    printf("\n======memoria de dados======\n");
    for (int i = 0; i < 16; i++) // linhas
    {
        for (int j = 0; j < 16; j++) // colunas
        {
            int idx = i * 16 + j;
            printf("[%3d] =%4d |",idx, mem[idx]);
        }

        printf("\n");
    }
}


void gerar_asm(instrucao p,int pc,char bin[]){
    FILE *arquivo;
    arquivo = fopen("assembly.asm","a");
    if (!arquivo){
        printf("\nProblema ao gerar arquivo!");
        return;
    }
    switch (p.opcode){
    case 0:
        switch (p.funct){
        case 0:
            fprintf(arquivo,"add $%d,$%d,$%d\n",p.rd,p.rs,p.rt);
            break;

        case 2:
            fprintf(arquivo,"sub $%d,$%d,$%d\n",p.rd,p.rs,p.rt);
            break;
        }
        break;

    case 2:
        fprintf(arquivo,"jump %d\n",p.addr);
        break;

    case 4:
        p.imm=sign_extend6to8(p.imm);
        fprintf(arquivo,"addi $%d,$%d,%d\n",p.rt,p.rs,p.imm);
        break;

    case 8:
        p.imm=sign_extend6to8(p.imm);
        fprintf(arquivo,"$%d,$%d,%d\n",p.rs,p.rt,p.imm);
        break;

    case 11:
        p.imm=sign_extend6to8(p.imm);
        fprintf(arquivo,"lw $%d,%d($%d)\n",p.rt,p.imm,p.rs);
        break;

    case 15:
        p.imm=sign_extend6to8(p.imm);
        fprintf(arquivo,"sw $%d,%d($%d)\n",p.rt,p.imm,p.rs);
        break;

    default:
        fprintf(arquivo,"instrucao desconhecida\n");
        break;
    }

    fclose(arquivo);
}
void gerar_dat(int memoria[])
{
    FILE *arq=fopen("arquivo_dados.txt","w");
    for (int  i = 0; i < 256; i++)
    {
        fprintf(arq,"%d\t",memoria[i]);
    }
    fclose(arq);
}

void mostrar_metricas(metricas m) {
    printf("\n\n---Métricas---"
    "\nInstruções executadas: %i"
    "\nInstruções tipo R executadas: %i"
    "\nInstruções tipo I executadas: %i"
    "\nInstruções tipo J executadas: %i",
    m.contInst, m.contInstReg, m.contInstImm, m.contInstJump);
    return;
}
void conversao(char bin[], int numero)
{
    int atual = numero;
    int indice = 15;   // 16 bits → começa no 15
    int resto = 0;

    // se for negativo trabalho como positivo
    if (numero < 0)
        atual = -numero;

    // conversão binária
    while (atual > 0 && indice >= 0)
    {
        resto = atual % 2;

        if (resto == 0)
            bin[indice] = '0';
        else
            bin[indice] = '1';

        atual /= 2;
        indice--;
    }

    // completa com zeros à esquerda
    while (indice >= 0)
    {
        bin[indice] = '0';
        indice--;
    }

    // complemento de dois se negativo
    if (numero < 0)
    {
        complemento2(bin);
    }

    bin[16] = '\0'; // final da string (16 bits)
}
void complemento2(char bin[])
{
    // inverte os bits
    for (int i = 0; i < 16; i++)
    {
        if (bin[i] == '0')
            bin[i] = '1';
        else
            bin[i] = '0';
    }

    // soma +1
    int carry = 1;

    for (int i = 15; i >= 0; i--)
    {
        if (bin[i] == '1' && carry == 1)
        {
            bin[i] = '0';
            carry = 1;
        }
        else if (bin[i] == '0' && carry == 1)
        {
            bin[i] = '1';
            carry = 0;
        }
    }
}
void imprimir_memoria(char memu[256][17], int m, int n, char* bin)
{
    int k = 0, j = 0;

    // ================= INSTRUÇÕES =================
    printf("\n================ MEMORIA DE INSTRUCAO ================\n");

    printf("+---------+------------------+------------------------------+\n");
    printf("| End     | Binario          | Assembly                     |\n");
    printf("+---------+------------------+------------------------------+\n");

    for (k = 0; k < 128 && k < m; k++) {

        printf("| %7d | ", k);

        // BINÁRIO ou vazio
        if (memu[k][0] == '\0') {
            printf("                  | ");
            printf("%-28s |\n", "");
            continue;
        }

        for (j = 0; j < n; j++) {
            printf("%c", memu[k][j]);
        }

        printf(" | ");

        // captura assembly sem quebrar linha visual
        imprimir_ass(bin, memu, k);

        printf("%*s|\n", 1, "");
    }

    printf("+---------+------------------+------------------------------+\n");


    // ================= DADOS =================
    printf("\n================ MEMORIA DE DADOS ================\n");

    printf("+---------+------------------+----------+\n");
    printf("| End     | Binario          | Decimal  |\n");
    printf("+---------+------------------+----------+\n");

    for (k = 128; k < 256 && k < m; k++) {

        printf("| %7d | ", k);

        if (memu[k][0] == '\0') {
            printf("                  | %8d |\n", 0);
            continue;
        }

        for (j = 0; j < n; j++) {
            printf("%c", memu[k][j]);
        }

        unsigned int valor = strtoul(memu[k], NULL, 2);

        printf(" | %8u |\n", valor);
    }

    printf("+---------+------------------+----------+\n");
}
controle sinais_controle_multiclo(int estado_atual)
{
    controle c;
    
    switch (estado_atual)
    {
    case 0:
        c.PCwrite=1;
        c.ALUOp=0;
        c.IouD=0;
        c.MemWrite=0;
        c.Irwrite=1;
        c.ula_fonteA=0;
        c.ula_fonteB=1;
        c.RegWrite=0;
        c.Branch=0;
        c.origPC=0;
        return c;
        break;
    case 1:
        c.PCwrite=0;
        c.MemWrite=0;
        c.Irwrite=0;
        c.ALUOp=0;
        c.ula_fonteA=0;
        c.ula_fonteB=2;
        c.Branch=0;
        c.RegWrite=0;
        return c;
    case 7:
        c.PCwrite=0;
        c.MemWrite=0;
        c.Irwrite=0;
        c.ALUOp=0;
        c.ula_fonteA=1;
        c.ula_fonteB=0;
        c.RegWrite=0;
        c.Branch=0;
        return c;
    case 8:
        c.RegDst=1;
        c.MemToReg=0;
        c.Branch=0;
        c.PCwrite=0;
        c.RegWrite=1;
        c.origPC=0;
        c.MemRead=0;
        c.Irwrite=0;
        return c;
    default:
        break;
    }
}
int mux_operandoA_ULA(controle c,int entrada1,int entrada2)
{
    switch (c.ula_fonteA)
    {
    case 0:
        printf("\nentrada 1:%d",entrada1);
        return entrada1;

        break;
    case 1:
        printf("\nentrada 2:%d",entrada2);
        return entrada2;
        break;
    default:
        break;
    }
}
int mux_operandoB_ULA(controle c,int entrada1,int entrada2,int entrada3)
{
    switch (c.ula_fonteB)
    {
    case 0:
        return entrada1;
        break;
    case 1:
        return entrada2;
        break;
    case 2:
        return entrada3;
        break;
    default:
        break;
    }
}
int mux_fontePC(controle c,int entrada1,int entrada2,int entrada3)
{
    switch (c.origPC)
    {
    case 0:
        return entrada1;
        break;
    case 1:
        return entrada2;
        break;
    case 2:
        return entrada3;
        break;
    default:
        break;
    }
}
int mux_memtoreg(controle c,int entrada1,int entrada2)
{
    switch (c.MemToReg)
    {
    case 0:
        printf("\nentrada 1:%d",entrada1);
        return entrada1;

        break;
    case 1:
        printf("\nentrada 2:%d",entrada2);
        return entrada2;
        break;
    default:
        break;
    }
}
int mux_IouD(controle c,int entrada1,int entrada2)
{
    switch (c.IouD)
    {
    case 0:
        printf("\nentrada 1:%d",entrada1);
        return entrada1;

        break;
    case 1:
        printf("\nentrada 2:%d",entrada2);
        return entrada2;
        break;
    default:
        break;
    }
}
int mux_RegDst(controle c,int entrada1,int entrada2)
{
    switch (c.RegDst)
    {
    case 0:
        printf("\nentrada 1:%d",entrada1);
        return entrada1;

        break;
    case 1:
        printf("\nentrada 2:%d",entrada2);
        return entrada2;
        break;
    default:
        break;
    }
}
void etapa_busca_multiciclo(int *estado_atual,char menu[256][17],int *pc,char RegIR[17])
{
    //primeiro passo é os sinais de controle para o estado 0
    int op1=0;
    int op2=0;
    int resultado_ula=0;
    int overflow=0;
    int zero=0;
    int saida_muxORIGpc=0;
    int resultado_and=0;
    int resultado_or=0;
    controle c;
    printf("\nEtapa de busca na organização multiclo!");
    printf("\nEstado atual da maquina de estado:%d",*estado_atual);
    c=sinais_controle_multiclo(*estado_atual);
    if (c.Irwrite==1)
    {
        //Se o sinal de escrita no registrador temporario de instrução de instrução
        //estiver ativado iremos escrever nele para isso iremos copiar a instruçao da memoria de instrucao para ele fazendo strcpy
        strcpy(RegIR, memu[*pc]);
        printf("\nInstruçãO salva em RI:%s",RegIR);
    }
    //agora iremos decidir os operandos fontes da ula
    op1=mux_operandoA_ULA(c,*pc,0);
    printf("\nOperando 1 ula:%d",op1);
    //observação a entrada 2 do mux para decidir o primeiro operando fonte é valor vindo dos registradores
    //todavia esse valor não foi feito a leitura dos registradores nessa etapa entao seria dont care entretando em c não tem como representar essa coisa
    op2=mux_operandoB_ULA(c,0,1,0);
    printf("\nOperando 2 ula:%d",op2);
    //O mesmo raciocinio vale para este mux visto que a primeira entrada vem do banco de regitradores e a 3 entrada
    //vem do sinal extendido do imediato todavia nesta etapa a instrução não foi decodificada portanto dont care
    resultado_ula=ula(op1,op2,c,&overflow,&zero);
    printf("\nresultado ula:%d",resultado_ula);
    saida_muxORIGpc=mux_fontePC(c,resultado_ula,0,0);
    //as duas entradas estao sendo representa como 0 para representar um dont care
    //visto que nessa etapa não escrevemos no registrador temporario saida da ula e nem utilizamos o valor do jump
    printf("\nentrada 1:%d",resultado_ula);
    printf("\nSaida mux fonte pc:%d",saida_muxORIGpc);
    //verificar se vai ter escrita no pc
    resultado_and=c.Branch & zero;
    printf("\nResultado and:%d",resultado_and);
    resultado_or=resultado_and | c.PCwrite;
    printf("\nResultado da OR:%d",resultado_or);
    if (resultado_or)
    {
        *pc=saida_muxORIGpc;
    }
    *estado_atual=1;
}
void etapa_decodificacao_multiciclo(int *estado_atual,char RegIR[17],int *Reg_aluout,int banco_reg[],int *reg_tempA,int *reg_tempB,int pc)
{
    controle c;
    instrucao i;
    int op1=0;
    int op2=0;
    int resultado_ula=0;
    int overflow=0;
    int zero=0;
    //primeiro iremos gerar os sinais de controle para este estado
    //estado atual é 1 que é o de decodificação
    c=sinais_controle_multiclo(*estado_atual);
    i = decodificar(RegIR);//decodificação
    printf("\ninstrucao em binario:%s",RegIR);
    imprimir_instrucao(i);
    //agora iremos pegar os valores no registradores e guarda nos registradores temporario
    *reg_tempA=registradores[i.rs];
    *reg_tempB=registradores[i.rt];
    i.imm=sign_extend6to8(i.imm);
    op1=mux_operandoA_ULA(c,pc,*reg_tempA);
    printf("\noperando operando 1:%d",op1);
    op2=mux_operandoB_ULA(c,*reg_tempB,1,i.imm);
    printf("\nvalor operando 2:%d",op2);
    resultado_ula=ula(op1,op2,c,&overflow,&zero);
    *Reg_aluout=resultado_ula;
    //agora iremos ver o proximo estado dependendo do opcode
    switch (i.opcode)
    {
    case 0:
        *estado_atual=7;
        break;
    case 4:
        *estado_atual=2;
        break;
    case 11:
        *estado_atual=2;
        break;
    case 15:
        *estado_atual=15;
        break;
    case 8:
        *estado_atual=9;
        break;
    case 2:
        *estado_atual=10;
        break;
    default:
        break;
    }
}
void etapa_execucao_tipoR(int *estado_atual,int reg_tempA,int reg_tempB,int *Reg_aluout)
{
    controle c;
    int op1=0,op2=0,resultado_ula=0;
    int overflow,zero;
    c=sinais_controle_multiclo(*estado_atual);
    op1=mux_operandoA_ULA(c,0,reg_tempA);
    printf("\nvalor operando 1:%d",op1);
    op2=mux_operandoB_ULA(c,reg_tempB,0,0);
    printf("\nvalor operando 2:%d",op2);
    resultado_ula=ula(op1,op2,c,&overflow,&zero);
    printf("\nvalor da ula:%d",resultado_ula);
    *Reg_aluout=resultado_ula;
    *estado_atual=8;
}
void etapa_termino_tipoR(int *estado_atual,int Reg_aluout,char RegIR[17],int banco_reg[])
{
    controle c;
    instrucao i;
    int saida_mux_memtoReg=0;
    int saida_mux_RegDST=0;
    c=sinais_controle_multiclo(*estado_atual);
    i = decodificar(RegIR);
    printf("\nregistrador RT:%d",i.rt);
    printf("\nregistrador RD:%d",i.rd);
    saida_mux_RegDST=mux_RegDst(c,i.rt,i.rd);
    printf("\nSaida mux registrador destino:%d",saida_mux_RegDST);
    saida_mux_memtoReg=mux_memtoreg(c,Reg_aluout,0);//essa parte eu vou arrumar depois
    printf("\nSaida mux memoria para o registrador:%d",saida_mux_memtoReg);
    banco_reg[saida_mux_RegDST]=saida_mux_memtoReg;
    *estado_atual=0;
}