Read me for formatting the log.csv file !!

Log.csv should be opened in Google Sheets or MS Excel (or a similar program) 
for proper readability.

Currently log.csv logs every 5 seconds while in the sampling/flushing states.

The first column of log.csv is the time. It is in unformatted Unix Epoch Time, 
meaning it shows up as a big number reprsenting the number of seconds since 1970.
To convert that big number into a human readable format, make a fourth column that
uses this formula:

=A1/86400+date(1970,1,1)

The second column is the current state of the machine - flushing, sampling, et cetera.

The third column is the pressure sensor reading. Presently it is zero.