// Autores: Regis de Abreu Barbosa               Numero USP: 3135701
//          Rodrigo Mendes Leme                  Numero USP: 3151151
// Curso: computacao                             Data: 1/10/2001
// Professor: JEF
// Exercicio Programa 1
// Compilador usado: GCC-Linux
// Descricao: simula o algoritmo desfazer/refazer de recuperacao de transacoes
//            em caso de falhas.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// *************** DECLARACAO DE CONSTANTES

#define TAM_CACHE 100
#define TAM_DISCO 10000
#define MAX_LINHAS_LOG 300
#define REFEITO 1
#define DESFEITO 2


// *************** DECLARACAO DE TIPOS

typedef struct _dir_cache
{
  int item_dado,      // Posicao do item de dado no disco
      posicao;        // Posicao do item de dado no cache
} Tdir_cache;

typedef struct _cache
{
  int bit_alteracao,     // 1 se o valor do item de dado foi alterado; 0 c. c.
      vazio;             // 1 se a posicao do cache estiver vazia; 0 c. c.
  char valor[20];        // Valor atual do item de dado
} Tcache;

typedef struct _lista_trans * ap_lista_trans;
typedef struct _lista_trans
{
  int transacao;
  ap_lista_trans prox;
} Tlista_trans;

typedef struct _log_aux
{
  int transacao, item_dado;
  char valor_ant[20];
} Tlog_aux;

typedef struct _lista_itens * ap_lista_itens;
typedef struct _lista_itens
{
  int item_dado;
  ap_lista_itens prox;
} Tlista_itens;

typedef struct _log
{
  int transacao,
      item_dado,
      tipo_recup;         // Indica se a transacao foi refeita ou desfeita
  char valor_ant[20],
       valor_novo[20],
       tipo;              // A == aborta, C == confirma, I = inicia,
} Tlog;                   // X == leitura/escrita


// *************** DECLARACAO DE VARIAVEIS GLOBAIS

Tdir_cache dir_cache[TAM_CACHE];
Tcache cache[TAM_CACHE];
char **disco;
Tlog log[MAX_LINHAS_LOG];
int ult_pos_log       = -1,  // Ultima posicao usada do log
    ult_pos_cache     = -1,  // Ultima posicao usada do cache (estrategia FIFO)
    ult_pos_dir_cache = -1;  // Ultima posicao usada do diretorio de cache
ap_lista_trans ativas      = NULL,     // Lista de transacoes ativas
               confirmadas = NULL,     // Lista de transacoes confirmadas
               abortadas   = NULL;     // Lista de transacoes abortadas


// *************** PROTOTIPOS DAS FUNCOES

void imprime_saida(void);
int procura_dir_cache(int posicao);
int procura_item_dado(int item_dado);
void atualiza_dir_cache(int posicao, int item_dado);
void descarrega(int posicao);
char *busca(int item_dado);
int pertence_lista(ap_lista_trans pri, int transacao);
void insere_lista(ap_lista_trans *pri, int transacao);
void remove_lista(ap_lista_trans *pri, int transacao);
int pertence(ap_lista_itens lista, int item_dado);
void uniao(ap_lista_itens *lista, int item_dado);
void adiciona_log_inicio(int transacao);
void adiciona_log_leitura(int transacao, int x, char *valor);
void adiciona_log_escrita(int transacao, int x, char *valor_ant,
                          char* valor_novo);
void adiciona_log_confirma(int transacao);
void adiciona_log_aborta(int transacao);
void GR_inicia(int transacao);
void GR_leitura(int transacao, int x);
void GR_escrita(int transacao, int x, char *v);
void GR_confirma(int transacao);
void GR_aborta(int transacao);
void GR_reinicia(void);


// *************** PROGRAMA PRINCIPAL

// Funcao: main
// Descricao: inicializa as variaveis de controle do programa e dispara a simu-
//            lacao do algoritmo de recuperacao.

int main(void)
{
  int i;

  disco = (char **) malloc(TAM_DISCO * sizeof(char *));
  for (i = 0; i < TAM_DISCO; i++)
    disco[i] = (char *) malloc(20 * sizeof(char));

  for (i = 0; i < TAM_DISCO; i++)     // Preenche o disco
    strcpy(disco[i],"VGDN");
  for (i = 0; i < TAM_CACHE; i++)     // "Esvazia" o cache
    cache[i].vazio = 1;

  strcpy(disco[1],"20");
  strcpy(disco[2],"0");
  strcpy(disco[3],"1000");
  strcpy(disco[4],"30");

  printf("\nEstado inicial do disco: %s %s %s %s\n",disco[1],disco[2],disco[3],
         disco[4]);

  GR_inicia(1);
  GR_inicia(2);
  GR_leitura(1,3);
  GR_escrita(1,3,"1100");
  GR_leitura(2,3);
  GR_confirma(1);
  GR_escrita(2,3,"1210");
  GR_confirma(2);
  GR_inicia(3);
  GR_leitura(3,4);
  GR_escrita(3,4,"25");
  GR_leitura(3,1);
  GR_escrita(3,1,"50");
  GR_confirma(3);
  GR_inicia(4);
  GR_leitura(4,2);
  GR_escrita(4,2,"10");
  GR_leitura(4,4);
  GR_escrita(4,4,"100");
  GR_aborta(4);
  GR_inicia(5);
  GR_leitura(5,4);
  GR_escrita(5,4,"3");
  GR_reinicia();

  printf("\nEstado final do disco: %s %s %s %s\n",disco[1],disco[2],disco[3],
         disco[4]);
  imprime_saida();
  return 0;
}


// Funcao: imprime_saida
// Descricao: imprime o log na tela.

void imprime_saida(void)
{
  int i;

  printf("\nArquivo de log:\n");
  for (i = 0 ; i <= ult_pos_log; i++)
    switch(log[i].tipo)
    {
      case 'A' : printf("(T%d, aborta)\n",log[i].transacao); break;
      case 'C' : printf("(T%d, confirma)\n",log[i].transacao); break;
      case 'X' : printf("(T%d, %d, %s, %s)\n",log[i].transacao,
                 log[i].item_dado,log[i].valor_ant,log[i].valor_novo);
                 break;
      case 'I' : printf("(T%d, inicia)\n",log[i].transacao); break;
    }

  printf("\nApos reiniciar o BD:\n");
  for (i = 0; i <= ult_pos_log; i++)
    if (log[i].tipo_recup == DESFEITO)
      printf("(T%d, %d, %s, %s) - desfeita\n",log[i].transacao,
             log[i].item_dado,log[i].valor_ant,log[i].valor_novo);
    else
      if (log[i].tipo_recup == REFEITO)
        printf("(T%d, %d, %s, %s) - refeita\n",log[i].transacao,
               log[i].item_dado,log[i].valor_ant,log[i].valor_novo);
}


// *************** FUNCOES DO CACHE

// Funcao: procura_dir_cache
// Entrada: posicao do cache a ser localizada.
// Saida: item de dado associado aquela posicao.
// Descricao: procura no diretorio de cache o item de dado associado a uma
//            posicao do cache.

int procura_dir_cache(int posicao)
{
  int i = 0;

  while (i < TAM_CACHE)
  {
    if (dir_cache[i].posicao == posicao)
      return dir_cache[i].item_dado;
    i++;
  }
  return -1;
}


// Funcao: procura_item_dado
// Entrada: item de dado a ser localizado.
// Saida: -1 se nao encontrou o item de dado; a posicao no cache caso contra-
//        rio.
// Descricao: procura no diretorio de cache um item de dado.

int procura_item_dado(int item_dado)
{
  int i = 0;

  while (i < TAM_CACHE)
  {
    if (dir_cache[i].item_dado == item_dado)
      if (!cache[dir_cache[i].posicao].vazio)
        return dir_cache[i].posicao;
    i++;
  }
  return -1;
}


// Funcao: atualiza_dir_cache
// Entrada: posicao do cache a ser localizada, item de dado a ser incluido.
// Descricao: procura no diretorio de cache a posicao do cache a ser atualizada
//            e altera o valor do item de dado associado.

void atualiza_dir_cache(int posicao, int item_dado)
{
  int i = 0;

  while (i < TAM_CACHE)
  {
    if (dir_cache[i].posicao == posicao)
    {
      dir_cache[i].item_dado = item_dado;
      break;
    }
    i++;
  }
}


// *************** FUNCOES DO GERENCIADOR DE MEMORIA

// Funcao: descarrega
// Entrada: posicao do cache que se quer descarregar.
// Descricao: descarrega a valor da posicao no disco, se este foi alterado.

void descarrega(int posicao)
{
  int item_dado;

  if (cache[posicao].bit_alteracao == 1)
  {
    item_dado = procura_dir_cache(posicao);
    strcpy(disco[item_dado],cache[posicao].valor);
    cache[posicao].bit_alteracao = 0;
  }
}


// Funcao: busca
// Entrada: item de dado a ser lido.
// Saida: valor do item de dado.
// Descricao: busca no cache ou no disco o valor de um item de dado.

char *busca(int item_dado)
{
  int posicao;

  posicao = procura_item_dado(item_dado);
  if (posicao != -1)         // O item de dado esta no cache
    return cache[posicao].valor;
  else
  {
    ult_pos_cache++;
    ult_pos_cache %= TAM_CACHE;      // Ultima posicao do cache, vai pro comeco
    if (!cache[ult_pos_cache].vazio)   // Aquela posicao do cache esta ocupada,
      descarrega(ult_pos_cache);       // deve descarrega-la
    else
    {
      ult_pos_dir_cache++;
      dir_cache[ult_pos_dir_cache].item_dado = item_dado;
      dir_cache[ult_pos_dir_cache].posicao   = ult_pos_cache;
    }
    cache[ult_pos_cache].bit_alteracao = 0;
    strcpy(cache[ult_pos_cache].valor,disco[item_dado]);       // "Le" do disco
    cache[ult_pos_cache].vazio = 0;
    atualiza_dir_cache(ult_pos_cache,item_dado);
    return cache[ult_pos_cache].valor;
  }
}


// Funcao: carrega_item_dado
// Entrada: item de dado a ser carregado.
// Saida: posicao no cache
// Descricao: carrega no cache o valor de um item de dado.

int carrega_item_dado(int item_dado)
{
  int posicao;

  posicao = procura_item_dado(item_dado);
  if (posicao != -1)         // O item de dado esta no cache
    return posicao;
  else
  {
    ult_pos_cache++;
    ult_pos_cache %= TAM_CACHE;      // Ultima posicao do cache, vai pro comeco
    if (!cache[ult_pos_cache].vazio)   // Aquela posicao do cache esta ocupada,
      descarrega(ult_pos_cache);       // deve descarrega-la
    else
    {
      ult_pos_dir_cache++;
      dir_cache[ult_pos_dir_cache].item_dado = item_dado;
      dir_cache[ult_pos_dir_cache].posicao   = ult_pos_cache;
    }
    cache[ult_pos_cache].bit_alteracao = 0;
    strcpy(cache[ult_pos_cache].valor,disco[item_dado]);       // "Le" do disco
    cache[ult_pos_cache].vazio = 0;
    atualiza_dir_cache(ult_pos_cache,item_dado);
    return ult_pos_cache;
  }
}


// *************** FUNCOES DAS LISTAS DE TRANSACOES

// Funcao: pertence_lista
// Entrada: uma lista ligada e uma transacao.
// Saida: 1 se a transacao pertence a essa lista; 0 caso contrario.
// Descricao: percorre a lista ate encontrar a transacao ou ate o fim.

int pertence_lista(ap_lista_trans pri, int transacao)
{
  ap_lista_trans atu;

  for (atu = pri; atu != NULL; atu = atu->prox)
    if (atu->transacao == transacao)
      return 1;
  return 0;
}


// Funcao: insere_lista
// Entrada: uma lista ligada e uma transacao.
// Descricao: insere na lista apontada por pri uma transacao.

void insere_lista(ap_lista_trans *pri, int transacao)
{
  ap_lista_trans novo;

  novo = (ap_lista_trans) malloc(sizeof(Tlista_trans));
  novo->transacao = transacao;
  novo->prox      = *pri;
  *pri            = novo;
}


// Funcao: remove_lista
// Entrada: uma lista ligada e uma transacao.
// Descricao: remove da lista apontada por pri uma transacao.

void remove_lista(ap_lista_trans *pri, int transacao)
{
  ap_lista_trans ant = NULL,
                 atu = *pri;

  while (atu != NULL)                   // Busca a transacao na lista
    if (atu->transacao == transacao)
      break;
    else
    {
      ant = atu;
      atu = atu->prox;
    }

  if (atu != NULL)          // Encontrou a transacao
  {
    if (atu == *pri)        // Trata-se do primeiro no da lista
    {
      free(atu);
      *pri = NULL;
    }
    else                    // Trata-se de um no interno da lista
    {
      ant->prox = atu->prox;
      free(atu);
    }
  }
}


// *************** FUNCOES DE CONJUNTOS

// Funcao: pertence
// Entrada: uma lista ligada e um item de dado.
// Saida: 1 se o item de dado pertence a lista; 0 caso contrario.
// Descricao: percorre a lista ate encontrar o item de dado ou ate o fim.

int pertence(ap_lista_itens lista, int item_dado)
{
  ap_lista_itens atu;

  for (atu = lista; atu != NULL; atu = atu->prox)
    if (atu->item_dado == item_dado)
      return 1;
  return 0;
}


// Funcao: uniao
// Entrada: uma lista ligada e um item de dado.
// Descricao:

void uniao(ap_lista_itens *lista, int item_dado)
{
  ap_lista_itens ant = NULL,
                 atu,
                 novo;

  for (atu = *lista; atu != NULL; atu = atu->prox)
  {
    if (atu->item_dado == item_dado)
      break;
    ant = atu;
  }
  if (atu == NULL)         // O item de dado nao esta na lista, deve incluir
  {
    novo            = (ap_lista_itens) malloc(sizeof(Tlista_itens));
    novo->item_dado = item_dado;
    novo->prox      = NULL;
    if (*lista == NULL)
      *lista = novo;
    else
      ant->prox = novo;
  }
}


// *************** FUNCOES DO LOG

// Funcao: adiciona_log_inicio
// Entrada: uma transacao.
// Descricao: escreve no log uma transacao de inicio.

void adiciona_log_inicio(int transacao)
{
  ult_pos_log++;
  log[ult_pos_log].transacao = transacao;
  log[ult_pos_log].tipo      = 'I';
}


// Funcao: adiciona_log_leitura
// Entrada: uma transacao, o item de dado associado e o valor desse item;
// Descricao: escreve no log uma transacao de leitura.

void adiciona_log_leitura(int transacao, int x, char *valor)
{
  adiciona_log_escrita(transacao,x,valor,valor);
}


// Funcao: adiciona_log_escrita
// Entrada: uma transacao, o item de dado associado, o valor antigo e o novo
//          valor.
// Descricao: escreve no log uma transacao de escrita.

void adiciona_log_escrita(int transacao, int x, char *valor_ant,
                          char* valor_novo)
{
  ult_pos_log++;
  log[ult_pos_log].transacao = transacao;
  log[ult_pos_log].item_dado = x;
  strcpy(log[ult_pos_log].valor_ant,valor_ant);
  strcpy(log[ult_pos_log].valor_novo,valor_novo);
  log[ult_pos_log].tipo = 'X';
}


// Funcao: adiciona_log_confirma
// Entrada: uma transacao.
// Descricao: escreve no log uma transacao de confirmacao.

void adiciona_log_confirma(int transacao)
{
  ult_pos_log++;
  log[ult_pos_log].transacao = transacao;
  log[ult_pos_log].tipo      = 'C';
}


// Funcao: adiciona_log_aborta
// Entrada: uma transacao.
// Descricao: escreve no log uma transacao de aborto.

void adiciona_log_aborta(int transacao)
{
  ult_pos_log++;
  log[ult_pos_log].transacao = transacao;
  log[ult_pos_log].tipo      = 'A';
}


// *************** FUNCOES DE RECUPERACAO

// Funcao: GR_inicia
// Entrada: uma transacao.
// Descricao: inicio da transacao.

void GR_inicia(int transacao)
{
  adiciona_log_inicio(transacao);
}


// Funcao: GR_leitura
// Entrada: uma transacao e o item de dado a receber o valor lido.
// Descricao: leitura do valor de x para a transacao.

void GR_leitura(int transacao, int x)
{
  char valor[20];

  strcpy(valor,busca(x));
  adiciona_log_leitura(transacao,x,valor);
}


// Funcao: GR_escrita
// Entrada: uma transacao, o item de dado a ser atualizado e o novo valor.
// Descricao: escrita do valor de v em x para a transacao.

void GR_escrita(int transacao, int x, char *v)
{
  char valor[20];
  int posicao;

  insere_lista(&ativas,transacao);
  strcpy(valor,busca(x));
  adiciona_log_escrita(transacao,x,valor,v);
  posicao = procura_item_dado(x);
  cache[posicao].bit_alteracao = 1;
  strcpy(cache[posicao].valor,v);
  descarrega(posicao);
}


// Funcao: GR_confirma
// Entrada: uma transacao.
// Descricao: confirmacao da transacao.

void GR_confirma(int transacao)
{
  insere_lista(&confirmadas,transacao);
  adiciona_log_confirma(transacao);
  remove_lista(&ativas,transacao);
}


// Funcao: GR_aborta
// Entrada: uma transacao.
// Descricao: cancelamento da transacao.

void GR_aborta(int transacao)
{
  int i,
      posicao;

  for (i = ult_pos_log; i >= 0; i--)        // Percorre do fim pro comeco o log
  {
    if ((log[i].transacao == transacao) && (log[i].tipo == 'X') &&
        (strcmp(log[i].valor_ant,log[i].valor_novo) != 0))
    {
      posicao = carrega_item_dado(log[i].item_dado);
      strcpy(cache[posicao].valor,log[i].valor_ant);
      cache[posicao].bit_alteracao = 1;
      descarrega(posicao);
    }
  }
  insere_lista(&abortadas,transacao);
  adiciona_log_aborta(transacao);
  remove_lista(&ativas,transacao);
}


// Funcao: GR_reinicia
// Descricao: inicia o processo de recuperacao no evento de uma falha.

void GR_reinicia(void)
{
  ap_lista_itens redone = NULL,
                 undone = NULL,
                 atu;
  ap_lista_trans atu_trans;
  int i,
      posicao;

  ult_pos_cache = ult_pos_dir_cache = -1;         // Descarta posicoes do cache
  for (i = 0; i < TAM_CACHE; i++)                 // "Esvazia" o cache
    cache[i].vazio = 1;

  // Percorre o log em busca de entradas do tipo [Ti,x,v] que nao pertencam
  // a redone U undone
  for (i = 0; i <= ult_pos_log; i++)
    if ((log[i].tipo == 'X') &&
      (strcmp(log[i].valor_ant,log[i].valor_novo) != 0) &&
      !pertence(redone,log[i].item_dado) && !pertence(undone,log[i].item_dado))
    {
      posicao = carrega_item_dado(log[i].item_dado);
      if (pertence_lista(confirmadas,log[i].transacao))
      {
        log[i].tipo_recup = REFEITO;
        strcpy(cache[posicao].valor,log[i].valor_novo);
        cache[posicao].bit_alteracao = 1;
        uniao(&redone,log[i].item_dado);
      }
      else
        if (pertence_lista(abortadas,log[i].transacao) ||
            pertence_lista(ativas,log[i].transacao))
        {
          log[i].tipo_recup = DESFEITO;
          strcpy(cache[posicao].valor,log[i].valor_ant);
          cache[posicao].bit_alteracao = 1;
          uniao(&undone,log[i].item_dado);
        }
    }

  // Para cada transacao confirmada, se ela estiver ativa "desativa-la"
  for (atu_trans = confirmadas; atu_trans != NULL; atu_trans = atu_trans->prox)
    if (pertence_lista(ativas,atu_trans->transacao))
      remove_lista(&ativas,atu_trans->transacao);

  // Algoritmo desfazer/refazer
  for (atu = redone; atu != NULL; atu = atu->prox)
    strcpy(disco[atu->item_dado],busca(atu->item_dado));      // Cache -> disco
  for (atu = undone; atu != NULL; atu = atu->prox)
    strcpy(disco[atu->item_dado],busca(atu->item_dado));      // Cache -> disco
}
