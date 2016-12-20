# razerChromaE1.31Listener
Listens for E1.31 (http://www.doityourselfchristmas.com/wiki/index.php?title=E1.31_(Streaming-ACN)_Protocol) and maps to razer chroma devices (keyboard, mousepad, mouse, etc)

Setup with multicast or unicast mode.

Quick Test:

	Edit mappings.xml to match your Razer Chroma hardware or run with what I have (kb, mouse, pad)
	Run chromaListener.exe in same directory with mappings.xml 
	Install & Run xLights
	Press Add E1.31
	Select Multicast or Unicast with 127.0.0.1 for IP Address
	Set last channel to 285 ( = 3*95 for RGB count of items I have mapped in mappings.xml, you hardware complement may vary)
	Save setup
	Tools/Test
	Check Controllers
	Select RBG Cycle tab and check mixed colors
	Check "Output to Lights" at top and the show should start

Updated 12/20/2016 - fixed many off by one issues, particularly with universes and propertys, wrote to use stl classes instead of arrays,  added multicast capability (made it the default).
