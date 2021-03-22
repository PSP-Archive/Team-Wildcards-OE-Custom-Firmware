<?php
	$DropSCMDB_sql = "
	DROP TABLE IF EXISTS SCMDB;
	DROP TABLE IF EXISTS PSP_GAME;
	DROP TABLE IF EXISTS PSP_CLASS;
	DROP TABLE IF EXISTS PSP_CODE;
	DROP TABLE IF EXISTS POP_GAME;
	DROP TABLE IF EXISTS POP_CLASS;
	DROP TABLE IF EXISTS POP_CODE;";
	$CreateSCMDB_sql = "
	CREATE TABLE SCMDB (
		PSP_GAME_num int(5),
		PSP_Class_num int(10),
		PSP_Code_num int(11),
		POP_GAME_num int(5),
		POP_Class_num int(10),
		POP_Code_num int(11),
		Last_updatetime datetime NOT NULL,
		Author varchar(12) NOT NULL default 'user',
		Maintainor varchar(50) NOT NULL default 'user, user'
	);
	CREATE TABLE PSP_GAME (
		ID int(5) NOT NULL auto_increment,
		GAME_ID varchar(12) NOT NULL,
		GAME_NAME nvarchar(50) NOT NULL,
		Region enum('Japan', 'America', 'Europe', 'Asia') NOT NULL,
		Content text NOT NULL,
		Class_num int(5) NOT NULL,
		Edit_datetime datetime NOT NULL,
		PRIMARY KEY  (ID)
	);
	CREATE TABLE PSP_CLASS (
		ID int(10) NOT NULL auto_increment,
		GAME_ID int(5) NOT NULL,
		Class_NAME nvarchar(50) NOT NULL,
		Approve enum('0', '1') NOT NULL default '0',
		Rate int(5) NOT NULL default 0,
		Content text NOT NULL,
		Code_num int(5) NOT NULL,
		Edit_datetime datetime NOT NULL,
		Editor nvarchar(200),
		PRIMARY KEY  (ID)
	);
	CREATE TABLE PSP_CODE (
		ID int(11) NOT NULL auto_increment,
		CLASS_ID int(10) NOT NULL,
		Code_NAME nvarchar(20) NOT NULL,
		Attr enum('0', '1', '2') NOT NULL,
		Code_0 varchar(8) NOT NULL,
		Code_1 varchar(8) NOT NULL,
		Code_2 varchar(8) NOT NULL,
		Code_3 varchar(8) NOT NULL,
		Edit_datetime datetime NOT NULL,
		PRIMARY KEY  (ID)
	);
	CREATE TABLE POP_GAME (
		ID int(5) NOT NULL auto_increment,
		GAME_ID varchar(12) NOT NULL,
		GAME_NAME nvarchar(50) NOT NULL,
		Region enum('Japan', 'America', 'Europe', 'Asia') NOT NULL,
		Content text NOT NULL,
		Class_num int(5) NOT NULL,
		Edit_datetime datetime NOT NULL,
		PRIMARY KEY  (ID)
	);
	CREATE TABLE POP_CLASS (
		ID int(10) NOT NULL auto_increment,
		GAME_ID int(5) NOT NULL,
		Class_NAME nvarchar(50) NOT NULL,
		Approve enum('0', '1') NOT NULL default '0',
		Rate int(5) NOT NULL default 0,
		Content text NOT NULL,
		Code_num int(5) NOT NULL,
		Edit_datetime datetime NOT NULL,
		Editor nvarchar(200),
		PRIMARY KEY  (ID)
	);
	CREATE TABLE POP_CODE (
		ID int(11) NOT NULL auto_increment,
		CLASS_ID int(10) NOT NULL,
		Code_NAME nvarchar(20) NOT NULL,
		Attr enum('0', '1', '2') NOT NULL,
		Code_0 varchar(8) NOT NULL,
		Code_1 varchar(8) NOT NULL,
		Code_2 varchar(8) NOT NULL,
		Code_3 varchar(8) NOT NULL,
		Edit_datetime datetime NOT NULL,
		PRIMARY KEY  (ID)
	);";
	$mysql_format_date = date("Y-m-d H:i:s");
	$InsertSCMDB = "INSERT INTO SCMDB VALUES(0, 0, 0, 0, 0, 0, '$mysql_format_date', 'user', 'user,user')";
?>