Read me for formatting and understanding the data.csv file

data.csv should be opened in Google Sheets or MS Excel (or a similar program) 
for proper readability.

Data.csv logs major milestones in the sampling routine.

The first column of data.csv is the time. It is in unformatted Unix Epoch Time, 
meaning it shows up as a big number reprsenting the number of seconds since 1970.
To convert that big number into a human readable format, make a fourth column that
uses this formula:

=A1/86400+date(1970,1,1)

The second column is the action performed.

Additional columns contain values from the sensors.