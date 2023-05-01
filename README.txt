What is this?
-------------
It's a cli ft8 decoder for multiple bands.

Will it transmit?
-----------------
Our grand ambition is to do so, yes. But for now, we'll settle for decoding
them all and displaying alerts.

How to use it?
--------------
Copy config.json.exmaple to config.json and edit it...
Ex:
	cp config.json{.example,}
	joe config.json

Start the mess:
	ft8cli

In theory, the client will keep the ft8 decoders running, check the logs
(ft8decoder.$band.log)
