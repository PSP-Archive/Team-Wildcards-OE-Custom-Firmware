<?php
/*
Extension Name: SCMDB
Extension Url: http://
Description: SCMDB.
Version: 1.0
Author: 
Author Url: http://     
*/
/*** Configuration settings ***/

import_request_variables("gp", "form_");

$Context->Dictionary['SCMDB'] = 'SCMDB';
$Context->Dictionary['SCMDBOptions'] = 'SCMDB Options';
$Context->Dictionary['SCMDB_Upload'] = 'Upload Codes';
$Context->Dictionary['SCMDB_Search'] = 'Search Codes';
$Context->Dictionary['SCMDB_Manage'] = 'Manage SCMDB';
$Context->Dictionary['PERMISSION_SCMDB_MANAGEMENT'] = 'Manage SCMDB';

$Context->Configuration['PERMISSION_SCMDB_MANAGEMENT'] = '0';

function my_implode($glue, array &$pieces, $offset, $num)
{
	$res = $pieces[$offset];
	for($i = 1; $i < $num; $i ++)
	{
		$res = $res . $glue . $pieces[$offset + $i];
	}
	return $res;
}

function deal_quotes($text)
{
	$tmp = htmlentities('"', ENT_QUOTES);
	$text = str_replace("'", '\\\'', $text);
	return str_replace('"', $tmp, $text);
}

function openSCMDB()
{
	$link = mysql_connect('mysql', 'user', 'pass', true);
	if(!$link)
	{
		die('Could not connect: ' . mysql_error());
	}
	mysql_select_db('scmdb', $link) or die(mysql_error());
	return $link;
}

function closeSCMDB($link)
{
	mysql_close($link) or die(mysql_error());
}

if (!array_key_exists('SCMDB_SETUP', $Configuration))
{
	$SCMDB_Link = openSCMDB();
	include 'initdb.php';
	$DropSCMDB = explode(";", $DropSCMDB_sql);
	for($i = 0; $i < 7; $i ++)
	{
		if (!mysql_query($DropSCMDB[$i], $SCMDB_Link)) {die('Could not query: ' . mysql_error());}
	}
	$CreateSCMDB = explode(";", $CreateSCMDB_sql);
	for($i = 0; $i < 7; $i ++)
	{
		if (!mysql_query($CreateSCMDB[$i], $SCMDB_Link)) {die('Could not query: ' . mysql_error());}
	}
	if (!mysql_query($InsertSCMDB, $SCMDB_Link)) {die('Could not query: ' . mysql_error());}
	closeSCMDB($SCMDB_Link);
	AddConfigurationSetting($Context, 'SCMDB_SETUP');
}

if(isset($Menu)) {
	$Menu->AddTab($Context->GetDefinition('SCMDB'), 'SCMDB', GetUrl($Configuration, 'extension.php', '', '', '', '', 'PostBackAction=SCMDB'), '','25');
}

if($Context->SelfUrl == 'extension.php' && in_array(ForceIncomingString("PostBackAction", ""), array('SCMDB', 'SCMDB_Upload', 'SCMDB_Upload_process', 'SCMDB_Search', 'SCMDB_Search_result', 'SCMDB_Manage')))
{
	$Head->AddStyleSheet('extensions/SCMDB/style.css');
	class SCMDB extends PostBackControl
	{
		var $Context;
		function SCMDB(&$Context)
		{
			$this->name = 'SCMDB';
			$this->ValidActions = array('SCMDB', 'SCMDB_Upload', 'SCMDB_Upload_process', 'SCMDB_Search', 'SCMDB_Search_result', 'SCMDB_Manage');
			$this->Constructor($Context);
		}
		function Render()
		{
			switch ($this->PostBackAction)
			{
				case 'SCMDB_Upload_process':
					$SCMDB_Link = openSCMDB();
					global $form_db;
					$form_db = str_replace("'", '\\\'', $form_db);
					global $form_code_type;
					global $form_uploadby;
					global $form_approve;
					$code = explode("\n", $form_db);
					$count = count($code);
					$i = 0;
					while($i < $count)
					{
						$code[$i] = trim($code[$i]);
						if(strstr($code[$i], 'ID:'))
						{
							$GAME_start = $i;
							$GAME_ID = substr($code[$i], 4);
							$Region = 'Japan';
							switch(substr($GAME_ID, 2, 1))
							{
								case "J":
									$Region = 'Japan';
									break;
								case "U":
									$Region = 'America';
									break;
								case "E":
									$Region = 'Europe';
									break;
								case "A":
									$Region = 'Asia';
									break;
							}
							while(!strstr($code[$i], 'NAME:'))
							{
								$i ++;
								$code[$i] = trim($code[$i]);
							}
							$GAME_NAME = substr($code[$i], 6);
							while(!strstr($code[$i], '$START'))
							{
								$i ++;
								$code[$i] = trim($code[$i]);
							}

							$Class_num = 0;

							$sql = "SELECT * FROM " . $form_code_type . "_GAME WHERE GAME_ID='$GAME_ID'";
							$result = mysql_query($sql, $SCMDB_Link) or die(mysql_error());
							$row = mysql_fetch_array($result);
							if(!$row)
							{
								$GAME_Content = my_implode("\\\\n", $code, $GAME_start, $i - $GAME_start + 1) . "\\\\n";
								$sql = "INSERT INTO " . $form_code_type . "_GAME (GAME_ID, GAME_NAME, Region, Content, Class_num) VALUES('$GAME_ID', '$GAME_NAME', '$Region', '$GAME_Content', 0)";
								mysql_query($sql, $SCMDB_Link) or die(mysql_error());
								$sql = "UPDATE SCMDB SET " . $form_code_type . "_GAME_num=" . $form_code_type . "_GAME_num+1";
								mysql_query($sql, $SCMDB_Link) or die(mysql_error());
								$sql = "SELECT * FROM " . $form_code_type . "_GAME WHERE GAME_ID='$GAME_ID'";
								$result = mysql_query($sql, $SCMDB_Link) or die(mysql_error());
								$row = mysql_fetch_array($result);
							}
							$G_ID = $row["ID"];
							while($i < $count)
							{
								if(strstr($code[$i], 'ID:'))
								{
									$i --;
									break;
								}
								if(strstr($code[$i], '{'))
								{
									$Class_start = $i;
									$code[$i] = trim($code[$i]);
									$Class_NAME = str_replace('{', ' ', substr($code[$i], 1));
									$Class_NAME = trim($Class_NAME);
									$sql = "SELECT * FROM " . $form_code_type . "_CLASS WHERE Class_NAME='$Class_NAME' AND GAME_ID=$G_ID";
									$result = mysql_query($sql, $SCMDB_Link) or die(mysql_error());
									$row = mysql_fetch_array($result);
									if($row)
									{
										$i ++;
										continue;
									}
									$Class_num ++;
									$Code_num = 0;
									$sql = "INSERT INTO " . $form_code_type . "_CLASS (GAME_ID, Class_NAME, Approve) VALUES($G_ID, '$Class_NAME', $form_approve)";
									mysql_query($sql, $SCMDB_Link) or die(mysql_error());
									$sql = "UPDATE SCMDB SET " . $form_code_type . "_Class_num=" . $form_code_type . "_Class_num+1";
									mysql_query($sql, $SCMDB_Link) or die(mysql_error());
									$sql = "SELECT * FROM " . $form_code_type . "_CLASS WHERE Class_NAME='$Class_NAME' AND GAME_ID=$G_ID";
									$result = mysql_query($sql, $SCMDB_Link) or die(mysql_error());
									$row = mysql_fetch_array($result);
									$C_ID = $row["ID"];
									while($i < $count)
									{
										if(strstr($code[$i], '('))
										{
											$Code_num ++;
											$code[$i] = trim($code[$i]);
											$Code_tmp = explode("$", substr($code[$i], 1));
											$Code_NAME = trim($Code_tmp[0]);
											$Attr = trim($Code_tmp[1]);
											$Codes = explode(" ", trim($Code_tmp[2], " ()}\t\n\r\0"));
											$sql = "INSERT INTO " . $form_code_type . "_CODE (CLASS_ID, Code_NAME, Attr, Code_0, Code_1, Code_2, Code_3, Edit_datetime) VALUES($C_ID, '$Code_NAME', '$Attr', '$Codes[0]', '$Codes[1]', '$Codes[2]', '$Codes[3]', '" . date("Y-m-d H:i:s") . "')";
											mysql_query($sql, $SCMDB_Link) or die(mysql_error());
											$sql = "UPDATE SCMDB SET " . $form_code_type . "_Code_num=" . $form_code_type . "_Code_num+1";
											mysql_query($sql, $SCMDB_Link) or die(mysql_error());
										}
										if(strstr($code[$i], '}'))
										{
											break;
										}
										$i ++;
									}
									$Class_Content = my_implode("\\\\n", $code, $Class_start, $i - $Class_start + 1);
									$sql = "UPDATE " . $form_code_type . "_CLASS SET Content='$Class_Content', Code_num=$Code_num, Edit_datetime='" . date("Y-m-d H:i:s") . "', Editor='$form_uploadby' WHERE ID=$C_ID";
									mysql_query($sql, $SCMDB_Link) or die(mysql_error());
								}
								$i ++;
							}
							$sql = "UPDATE " . $form_code_type . "_GAME SET Class_num=Class_num+$Class_num, Edit_datetime='" . date("Y-m-d H:i:s") . "' WHERE ID=$G_ID";
							mysql_query($sql, $SCMDB_Link) or die(mysql_error());
							$sql = "UPDATE SCMDB SET Last_updatetime='" . date("Y-m-d H:i:s") . "'";
							mysql_query($sql, $SCMDB_Link) or die(mysql_error());
						}
						$i ++;
					}
					closeSCMDB($SCMDB_Link);
					echo "<div>Upload successfully!</div>";
					break;
				case 'SCMDB':
					$SCMDB_Link = openSCMDB();
					$sql = "SELECT * FROM SCMDB";
					$result = mysql_query($sql);
					closeSCMDB($SCMDB_Link);
					$row = mysql_fetch_row($result);
					echo "<div>There is $row[0] PSP games($row[1] classes, $row[2] codes), $row[3] POP games($row[4] classes, $row[5] codes) in SCM Database, last updated in $row[6]<br>Powered by, Designed by, , Maintainor: $row[8]</div>";
					break;
				case 'SCMDB_Upload':
					$uploadby = 'unknown';
					if ($this->Context->Session->UserID > 0)
					{
						$uploadby = $this->Context->Session->User->Name;
					}
					echo '
					<div id="Form" class="Account SCMDB">
						<fieldset>
							<legend>Add SCM CODE</legend>
							<form class="SCMDB" method=post action="extension.php?PostBackAction=SCMDB_Upload_process" onSubmit="return checkForm();">
								<div id = "SCMDB_Upload_Form">
									<ul>
										<li>
											<label>Upload by:</label>
										    <input type="text" name="uploadby" value="'.$uploadby.'" maxlength="20"/>
										</li>';
										if($this->Context->Session->User->Permission('PERMISSION_SCMDB_MANAGEMENT'))
										{
											echo'
											<label>Aprovement:</label>
											<li>
											    <select name="approve">
												  <option value = 1>Not approved</option>
												  <option value = 0>Approved</option>
												</select>
											</li>';
										}
										echo '
										<li>
											<label>Code type:</label>
											<select name="code_type">
											  <option value = "PSP">PSP</option>
											  <option value = "POP">POP</option>
											</select>
										<li>
											<label>Copy your code here</label>
											<textarea rows="60" cols="45" name="db" wrap=off></textarea><br>
										</li>
										<li>
											<input type="submit" name="upload" value="upload" />
										</li>
									</ul>
								</div>
							</form>
						</fieldset>
					</div>';
					break;
				case 'SCMDB_Search':
					echo '
					<div id="Form" class="Account SCMDB">
						<fieldset>
							<legend>Search SCM Database</legend>
							<form class="SCMDB" method=post action="extension.php?PostBackAction=SCMDB_Search_result" onSubmit="return checkForm();">
								<div id = "SCMDB_Search_Form">
									<ul>
										<li>
											<select name="search_type" style="width: 90px">
											  <option value = "GAME_ID">GAME ID</option>
											  <option value = "GAME_NAME">GAME NAME</option>
											</select>
											<input type="text" name="search_text" value="" maxlength="50" style="width: 300px"/>
										</li>
										<li>
											Code type: 
											<select name="code_type" style="width: 100px; margin-right: 20px;">
											  <option value = "PSP">PSP</option>
											  <option value = "POP">POP</option>
											</select>
											Aprovement:
											<select name="approved" style="width: 100px">
											  <option value = 2>ALL</option>
											  <option value = 1>Approved</option>
											  <option value = 0>Not Approved</option>
											</select>
										</li>
										<li>
											<input type="submit" name="search" value="search" />
										</li>
									</ul>
								</div>
							</form>
						</fieldset>
					</div>';
					break;
				case 'SCMDB_Search_result':
					$SCMDB_Link = openSCMDB();
					global $form_code_type;
					global $form_search_type;
					global $form_search_text;
					$form_search_text = str_replace("'", '\\\'', $form_search_text);
					global $form_approved;
					global $form_unapproved;
					if($form_search_text)
					{
						$sql = "SELECT * FROM " . $form_code_type . "_GAME WHERE $form_search_type LIKE '%$form_search_text%'";
					}
					else
					{
						$sql = "SELECT * FROM " . $form_code_type . "_GAME";
					}
					$result = mysql_query($sql, $SCMDB_Link) or die(mysql_error());
					$game_count = 0;
					$game[$game_count] = mysql_fetch_array($result);
					while($game[$game_count])
					{
						$game_count ++;
						$game[$game_count] = mysql_fetch_array($result);
					}
					if($game_count == 0)
					{
						echo 'No Result!';
						break;
					}
					echo '
					<div class="SCMDB_result">
						<div class="SCMDB_result_left">';
					for($i = 0; $i < $game_count; $i ++)
					{
						if($form_approved < 2)
						{
							$sql = "SELECT * FROM " . $form_code_type . "_CLASS WHERE GAME_ID = " . sprintf("%d", $game[$i]['ID']) . " AND Approve = '$form_approved'";
						}
						else
						{
							$sql = "SELECT * FROM " . $form_code_type . "_CLASS WHERE GAME_ID = " . sprintf("%d", $game[$i]['ID']);
						}
						$result = mysql_query($sql, $SCMDB_Link) or die(mysql_error());
						$class_count[$i] = 0;
						$class[$i][$class_count[$i]] = mysql_fetch_array($result);
						while($class[$i][$class_count[$i]])
						{
							$class_count[$i] ++;
							$class[$i][$class_count[$i]] = mysql_fetch_array($result);
						}
						echo '
						<div onmouseover="document.getElementById(\'game' . sprintf("%d", $i) . '\').className=\'SCMDB_result_game SCMDB_visible\'" onmouseout="document.getElementById(\'game' . sprintf("%d", $i) . '\').className=\'SCMDB_result_game SCMDB_hidden\'">
						<h2 onClick="document.getElementById(\'resultArea\').value = \'' . deal_quotes($game[$i]['Content']); 
						for($j = 0; $j < $class_count[$i]; $j ++)
						{
							echo deal_quotes($class[$i][$j]['Content']) . "\\n\\n";
						}
						echo '\'">
							'. $game[$i]['GAME_ID'] . ' ' . $game[$i]['GAME_NAME'] . '
						</h2>
						<ul id="game' . sprintf("%d", $i) . '" class="SCMDB_result_game SCMDB_hidden">';
							for($j = 0; $j < $class_count[$i]; $j ++)
							{
								echo '
								<li onClick="document.getElementById(\'resultArea\').value = \'' . deal_quotes($game[$i]['Content']); 
								echo deal_quotes($class[$i][$j]['Content']);
								echo '\'">
									'. $class[$i][$j]['Class_NAME'] . '
								</li>';
							}
						echo '
						</ul>
						</div>';
					}
					echo '
						</div>
						<div class="SCMDB_result_content">
							<textarea id="resultArea" rows="45" cols="50" name="resultArea" wrap=off></textarea>
						</div>
					</div>';
					closeSCMDB($SCMDB_Link);
					break;
			}
		}
	}
	
	
	$SCMDBOptions = $Context->GetDefinition("SCMDBOptions");
	$Panel->AddList($SCMDBOptions, $Position = '10', $ForcePosition = '1');
	$Panel->AddListItem($SCMDBOptions, $Context->GetDefinition('SCMDB_Upload'), GetUrl($Context->Configuration, 'extension.php', '', '', '', '', 'PostBackAction=SCMDB_Upload'));
	$Panel->AddListItem($SCMDBOptions, $Context->GetDefinition('SCMDB_Search'), GetUrl($Context->Configuration, 'extension.php', '', '', '', '', 'PostBackAction=SCMDB_Search'));
	if($Context->Session->User->Permission('PERMISSION_SCMDB_MANAGEMENT'))
	{
		$Panel->AddListItem($SCMDBOptions, $Context->GetDefinition('SCMDB_Manage'), GetUrl($Context->Configuration, 'extension.php', '', '', '', '', 'PostBackAction=SCMDB_Manage'));
	}
	
	$Context->PageTitle = $Context->GetDefinition('SCMDB');
	$Menu->CurrentTab = 'SCMDB';
	$SCMDBobj = $Context->ObjectFactory->NewContextObject($Context, 'SCMDB');
	$Page->AddRenderControl($SCMDBobj, $Configuration["CONTROL_POSITION_BODY_ITEM"]);
}

?>