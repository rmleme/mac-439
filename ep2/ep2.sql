/**************************************************************** 
 * EP 2 de MAC 439						*
 *								*
 * Alunos: Regis de Abreu Barbosa    N.USP: 3135701		*
 *         Rodrigo Mendes Leme              3151151		*
 *								*
 * Observação: Para rodar SPs dos exercícios propostos		*
 * - exercício 1:						*
 *     exec sp_leme_001 @RET output				*
 * - exercício 2:						*
 *     exec sp_leme_002						*
 * - exercício 3:						*
 *     exec sp_leme_003 @RET output				*
 * sendo @RET numeric(10).					*
 ****************************************************************/

use labbd
go
create table identity_pri (
	ID numeric(10),
	NOME char(20),
	primary key (ID))
go
create table pessoa_pri (
	ID_PESSOA numeric(10),
	NOME char(10),
	TP_PESSOA int,
	primary key (ID_PESSOA))
go
create table pessoa_fisica_pri (
	ID_PESSOA numeric(10),
	DOCUMENTO char(15),
	NOME_PAI char(10),
	NOME_MAE char(10),
	primary key (ID_PESSOA))
go
create table pessoa_juridica_pri (
	ID_PESSOA numeric(10),
	ID_SETOR_ATIVIDADE numeric(10),
	DOCUMENTO char(12),
	INI_ATIVIDADE char(10),
	FIM_ATIVIDADE char(10),
	MOTIVO char(100),
	NOME_FANTASIA char(100),
	primary key (ID_PESSOA))
go
create table socio_pri (
	ID_PESSOA numeric(10),
	PES_ID_PESSOA numeric(10),
	primary key (ID_PESSOA, PES_ID_PESSOA))
go
create table setor_atividade_min (
	ID_SETOR_ATIVIDADE numeric(10),
	NOME_SETOR char(20),	
	DESCRICAO char(100),
	primary key (ID_SETOR_ATIVIDADE))
go

create procedure sp_get_id (@NOME char(20), @ID numeric(10) output) as
begin
begin transaction
	update identity_pri
	set ID = ID + 1
	where NOME = @NOME
	select @ID = ID from identity_pri where NOME = @NOME
	if (@@ROWCOUNT = 0)
	begin
		insert into identity_pri values (1, @NOME)
		select @ID = 1
	end
	if (@@ERROR = 0)
		commit
	else
		rollback
commit transaction
end
go
/* sp: spi_leme_001
 * Inserir pessoa fisica se esta nao existir. 
 * Retorna ID da pessoa
 */
create procedure spi_leme_001 (@NOME char(10), @DOCUMENTO char(15), @NOME_PAI char(10), @NOME_MAE char(10), @ID numeric(10) output) as
begin
begin transaction
        if (@ID = NULL)
		select @ID = 1
	select * from pessoa_pri where ID_PESSOA = @ID
	if (@@ROWCOUNT = 0)
	begin
		select NOME from pessoa_pri where NOME = @NOME
		if (@@ROWCOUNT = 0)
		begin
			insert into pessoa_pri values (@ID, @NOME, 1)
			insert into pessoa_fisica_pri
			values (@ID, @DOCUMENTO, @NOME_PAI, @NOME_MAE)
		end 
	commit transaction
	end
	select @ID = @ID + 1
	rollback transaction
end
go

/* sp: spi_leme_002
 * Inserir pessoa juridica se esta nao existir. 
 * Retorna ID da pessoa
 */

create procedure spi_leme_002 (@NOME char(10), @ID_SETOR_ATIVIDADE numeric(10), @DOCUMENTO char(12), @MOTIVO char(100), @NOME_FANTASIA char(100), @ID numeric(10) output) as
begin
begin transaction
	if (@ID = null)
	begin
		select @ID = 1
	end
	select * from pessoa_pri where ID_PESSOA >= @ID
	if (@@ROWCOUNT = 0)
	begin
	select NOME from pessoa_pri where NOME = @NOME
		if (@@ROWCOUNT = 0)
		begin
			insert into pessoa_pri values (@ID, @NOME, 2)
insert into pessoa_juridica_pri
			values (@ID, @ID_SETOR_ATIVIDADE, @DOCUMENTO, "", "", @MOTIVO, @NOME_FANTASIA)
		end
	commit transaction
	end
	select @ID = @ID + 1
	ROLLBACK transaction
end
go

/* sp: spi_leme_003
 * Inserir socio. 
 * Retorna 1 se inserir socio, 0 caso contrario.
 */
create procedure spi_leme_003 (@NOME_SOCIO char(10), @NOME_EMPRESA char(10), @RET numeric(10) output) as
begin
begin transaction	
	declare @ID_SOCIO numeric(10)
	declare @ID_EMPRESA numeric(10)
	select @RET = 0
	select @ID_SOCIO = ID_PESSOA from pessoa_pri where NOME = @NOME_SOCIO
	if (@@ROWCOUNT = 0)
	begin
		commit transaction
	end
	select @ID_EMPRESA = ID_PESSOA
	from pessoa_pri
	where NOME = @NOME_EMPRESA and TP_PESSOA = 2
	if (@@ROWCOUNT = 0)
	begin
		commit transaction
	end
	select @RET = ID_PESSOA 
	from socio_pri 
	where ID_PESSOA = @ID_SOCIO and PES_ID_PESSOA = @ID_EMPRESA
	if (@@ROWCOUNT = 0)
	begin
	   insert into socio_pri values (@ID_SOCIO, @ID_EMPRESA)
	end
commit transaction
end
go
/* sp: spa_leme_004
 * Remover socio. 
 * Retorna ID do socio.
 */
create procedure spa_leme_004 (@NOME_SOCIO char(10), @NOME_EMPRESA char(10), @RET numeric(10) output) as
begin
begin transaction
	delete from socio_pri
	where ID_PESSOA = (select ID_PESSOA
			   from pessoa_pri
			   where NOME = @NOME_SOCIO)
		and PES_ID_PESSOA = (select ID_PESSOA
				     from pessoa_pri
				     where NOME = @NOME_EMPRESA)
commit transaction
end
go

/* sp: sp_leme_001
 * Exercicio 1
 */
create procedure sp_leme_001 (@RET numeric(10) output) as
begin 
declare @ID numeric(10), @setor1 numeric(10), @setor2 numeric(10)
	exec spi_leme_001 "Joao", "0001", "Joao Pai", "Joana", @ID output 
	exec spi_leme_001 "Pedro", "0002", "Pedro Pai", "Mae do Pedro", @ID output
	exec spi_leme_002 "IME USP", 45, "Matematica", "undef", "undef", @ID output
	exec spi_leme_002 "Nova Castelo", 50, "Engenharia", "", "", @ID output
	exec spi_leme_003 "Joao", "Nova Castelo", @ID output
	exec spi_leme_003 "Pedro", "Nova Castelo", @ID output
	exec spi_leme_003 "IME USP", "Nova Castelo", @ID output
end
go

/* sp: sp_leme_002
 * Exercicio 2
 */
create procedure sp_leme_002 as
begin 
begin transaction
       select PESSOA_PRI.NOME from PESSOA_PRI, SOCIO, PESSOA_JURIDICA_PRI
       where SOCIO.ID_PESSOA in (select ID_PESSOA from PESSOA_JURIDICA_PRI)
	     and SOCIO.PES_ID_PESSOA = PESSOA_JURIDICA_PRI.ID_PESSOA
	     and PESSOA_PRI.ID_PESSOA = SOCIO.ID_PESSOA
       group by PESSOA_PRI.ID_PESSOA
       having count (distinct PESSOA_JURIDICA_PRI.ID_SETOR_ATIVIDADE) >= 2
commit transaction
end 
go

/* sp: sp_leme_003
 * Exercicio 3
 */
create procedure sp_leme_003 (@RET numeric(10) output) as
begin 
	declare @ID numeric(10)
	select @ID = 5
	exec spi_leme_001 "Ana", "0003", "Ana Pai", "Ana Mae", @ID output
	exec spi_leme_001 "Carol", "0004", "Carol Pai", "Carol Mae", @ID output

	exec spa_leme_004 "Joao", "Nova Castelo", @ID output
	exec spa_leme_004 "Pedro", "Nova Castelo", @ID output
	exec spi_leme_003 "Ana", "Nova Castelo", @ID output
	exec spi_leme_003 "Carol", "Nova Castelo", @ID output
end
go


declare @RET numeric(10)
exec sp_leme_001 @RET output

exec sp_leme_002

declare @RET numeric(10)
exec sp_leme_003 @RET output