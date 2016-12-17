#!/usr/bin/python

import os, cgi
import sys
import MySQLdb
import mysqlpw     # hold user name and password for mysql database

print "Content-Type: text/html\n"


# fetch parameters of GET callv ########################################
query     = os.environ.get("QUERY_STRING", "No Query String in url\n")
arguments = cgi.parse_qs(query) 

tab   = arguments.get('tab')
temp  = arguments.get('temp')
press = arguments.get('press')
hum   = arguments.get('hum')


# check if table name is defined #######################################
if arguments.has_key('tab'):
	None
	print  str(tab)[2:-2] + "<br />"
else:
	print "No table name defined. Use 'sqlentry.py?tab=sensordata_brunnen&temp=20' or similar.<br />"
	sys.exit()

	
# collect values #######################################################
sql_column = " ("
sql_value = " VALUES ("

if arguments.has_key('temp'):
	sql_column = sql_column + "temperatur,"
	sql_value = sql_value + str(temp)[2:-2] + ","
if arguments.has_key('press'):
	sql_column = sql_column + "luftdruck,"
	sql_value = sql_value + str(press)[2:-2] + ","
if arguments.has_key('hum'):
	sql_column = sql_column + "feuchtigkeit,"
	sql_value = sql_value + str(hum)[2:-2] + ","

	
# check if any value name was found. Escape script if not	
if sql_column[-1:] == ",":
	sql_column = sql_column[:-1] + ")"
	sql_value = sql_value[:-1] + ");"
else:
	print "No value defined. Use 'sqlentry.py?tab=sensordata_brunnen&temp=20' or similar.<br />"
	print "Permitted value names are: 'temp', 'press' and 'hum'. At least one must be specified.<br />"
	sys.exit()

# assemble mySQL command string
sql_command = "INSERT INTO " + str(tab)[2:-2] + sql_column + sql_value
print sql_command + "<BR />"


# write data to database #################################################	
try:
	db = MySQLdb.connect(host="localhost",    # your host, usually localhost
						user=mysqlpw.username,      # username of physical sensors
						passwd=mysqlpw.password,    # password
						db="datalog")         # dump database for sensor data
except:
	print "Database connection failed.<BR />"

	
try:	
	# Cursor object
	cur = db.cursor()

	# use cursor
	cur.execute(sql_command)
	db.commit()
	db.close()

except:
	print "Database command string failed.<BR />"
	db.rollback()
	db.close()
	sys.exit()

























