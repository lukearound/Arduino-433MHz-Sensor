#!/usr/bin/python

# http://bokeh.pydata.org/en/latest/docs/gallery/stocks.html

import MySQLdb
import pandas as pd
import numpy as np
import mysqlpw
from bokeh.layouts import gridplot
from bokeh.plotting import figure, show, output_file
from bokeh.models import DatetimeTickFormatter

print "lib import fertig"



def convert_sql_to_df(rows):
	#df = pd.DataFrame( pd.to_datetime([n[1] for n in rows]), (m[0] for m in rows), columns=['A','B'] );
	df = pd.DataFrame( [[ij for ij in i] for i in rows] ); 
	df.rename(columns={0: 'temp1', 1: 'datetime'}, inplace=True);
	df.set_index('datetime');
	df['datetime'] = [df_.tz_localize('Europe/Berlin') for df_ in df['datetime']];
	df = df.sort(['datetime'], ascending=[1]);
	#df['temp1'] = df['temp1'].interpolate(method='cubic');
	
	# average data to smooth plot
	df['temp1'] = pd.rolling_mean(df['temp1'],window=12);
	# drop NaN from rolling window
	#df = df.dropna();
	return df
	


	
db = MySQLdb.connect(host="localhost",    # your host, usually localhost
                     user=mysqlpw.username,     # username of physical sensors
                     passwd=mysqlpw.password,   # password
                     db="datalog")     # dump database for sensor data

# Cursor object
cur1 = db.cursor()
cur2 = db.cursor()
cur3 = db.cursor()

# use cursor
cur1.execute("SELECT `temperatur`,`datetime` FROM `sensordata_carport` ORDER BY `datetime` DESC LIMIT 5000")
cur2.execute("SELECT `temperatur`,`datetime` FROM `sensordata_brunnen` ORDER BY `datetime` DESC LIMIT 5000")
cur3.execute("SELECT `temperatur`,`datetime` FROM `sensordata_garage` ORDER BY `datetime` DESC LIMIT 5000")
db.close()


# preapre data
rows1 = cur1.fetchall()
rows2 = cur2.fetchall()
rows3 = cur3.fetchall()

print "sql data import fertig"

df1 = convert_sql_to_df(rows1)
df2 = convert_sql_to_df(rows2)
df3 = convert_sql_to_df(rows3)

print "sql data Konvertierung fertig"





# create plot
p1 = figure(x_axis_type="datetime", title="Temperaturverlauf", tools="pan,wheel_zoom,box_zoom,reset,hover")
p1.grid.grid_line_alpha=0.95
p1.xaxis.axis_label = 'Date'
p1.yaxis.axis_label = 'C'

p1.xaxis.formatter=DatetimeTickFormatter(minutes="%H:%M", hours="%H:%M", days="%d.%b", months="%b", years="%Y")

#dict(
#        hours=["%H"],
#        days=["%d"],
#        months=["%B"],
#        years=["%Y"],
#    ))




p1.line(df1['datetime'], df1['temp1'], color='#3288bd', legend='Carport', line_width=2, line_alpha=1.0)
p1.line(df2['datetime'], df2['temp1'], color='#99d594', legend='Brunnen', line_width=2, line_alpha=1.0)
p1.line(df3['datetime'], df3['temp1'], color='#fc8d59', legend='Garage', line_width=2, line_alpha=1.0)
p1.legend.location = "top_left"

output_file("/var/www/html/index.html", title="Temperaturverlauf")

show(gridplot([[p1]], plot_width=800, plot_height=500))








#