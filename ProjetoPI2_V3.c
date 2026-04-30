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
} instrucao;

//struct para decodificação
typedef struct unidade_controle {
    int RegDst;
    int ALUSrc;
    int MemToReg;
    int RegWrite;
    int MemRead;
    int MemWrite;
    int Branch;
    int ALUOp;
    int jump;
}controle;

typedef struct metricas {
    int contInst;
    int contInstReg;
    int contInstImm;
    int contInstJump;
} metricas;


int registradores[8]={0, 0, 0, 0, 0, 0, 0, 0};
int memoria[256] = {0};
int oldreg[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int oldmem[256] = {0};
int oldpc=0;
char memu[256][17];

instrucao decodificar(char *bin);
void imprimir_ass (char*bin, char memu[256][17], int k);
void carregamem (char memu[256][17], int m, int n);
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


int main() {
    FILE *mem = NULL;
    char **mem_instr = NULL;
    int m = 256;
    int n = 17;
    int escolha=1,pc=0;
    char bin[17];
    instrucao i;
    controle c;
    metricas metricas = {0};
    mem_instr = criameminstr(m, n);
    int temp_pc=0;
    
    printf("\n\nMenu de opcoes do programa");
    do { printf("\n\n[1] Carregar memoria de instrucao");
     printf("\n[2] Carregar memoria de dados");
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
             carregamem(memu, m, n);
             break;
         case 2: 
            carregadat(memoria);
         break;
         case 3:
            imprimir_mem_instr(memu,m,n, bin);
            imprimir_mem_dados(memoria);
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

void carregamem (char memu[256][17], int m, int n){
  char arq[256];
  setbuf(stdin, NULL);
  printf("Digite o nome do arquivo a ser lido: ");
  scanf("%s", arq);
  mem = fopen(arq, "r");
  if (mem == NULL){
    printf("Erro ao abrir o arquivo!\n");
    return;
      }
    int i=0;
    char aux[32];
    while(fgets(aux, sizeof(aux), mem) != NULL && i<128){
    aux[strcspn(aux, "\n")]= '\0';
    if(strcmp(aux, ".data")==0){
      break;
    }
      strcpy(memu[i], aux);
    i++;
    }
     fclose(mem);
    printf("Memoria carregada!\n");
    return;
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
    int indice = 7;
    int resto=0;

    // se for negativo eu trabalho como se fosse positivo para depois eu aplica o complemento de dois
    if (numero < 0)
        atual = -numero;

    // converte normalmente
    while (atual > 0 && indice >= 0)
    {
        resto=atual % 2;
        if (resto==0)
        {
            bin[indice]='0';
        }
        else
        {
            bin[indice]='1';
        }
        atual /= 2;
        indice--;
    }

    while (indice >= 0)
    {
        bin[indice] = '0';
        indice--;
    }

    // se o numero for negativo eu aplico o complemento de dois
    if (numero < 0)
    {
        complemento2(bin);
    }

    bin[8] = '\0';
}

void complemento2(char bin[])
{
    //aqui eu inverto os bits
    for (int i = 0; i < 8; i++)
    {
        if (bin[i] == '0')
            bin[i] = '1';
        else
            bin[i] = '0';
    }

    //aqui eu somo +1
    int carry = 1;

    for (int i = 7; i >= 0; i--)
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