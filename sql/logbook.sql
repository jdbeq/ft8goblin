create table logbook (
   qso_id INTEGER PRIMARY KEY AUTOINCREMENT,
   qso_call VARCHAR(24),
   qso_gridsquare VARCHAR(8),
   qso_mode VARCHAR(16),
   qso_rst_send INTEGER,
   qso_rst_rcvd INTEGER,
   qso_date TEXT,
   qso_time_on TEXT,
   qso_date_off TEXT,
   qso_time_off TEXT,
   qso_band TEXT,
   qso_freq REAL,
   qso_station_callsign TEXT,
   qso_station_gridsquare TEXT
);
