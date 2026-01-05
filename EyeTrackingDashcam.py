import cv2
import threading
import time
import numpy as np
import queue
import os
import sys
from gaze_tracking import GazeTracking

# Global variable to store the captured frame
frame = None
frame_lock = threading.Lock()

# Gaze tracking variables
Looking_at_roadP = True
xupperlimit = 0.7
xlowerlimit = 0.4
yupperlimit = 0.7
ylowerlimit = 0.4
starttime = time.time()
timewatchingroad = 0
timenotwatchingroad = 0
elapsedtime = 0
face_foundP = False
debounce = False
timelookingatroad = 0
timenotlookingatroad = 0
Previous_Time = 0
starttime = time.time()
timepressed = False
Exit = False

Session_Started = False
Session_Timer = 0
Session_Paused = False
Paused_Time = 0
Pause_Timer = 0
firsttime = True

PastSession_Timer = 0
Pasttimelookingatroad = 0
Pasttimenotlookingatroad = 0
ClearTab = False

# A counter to decide when to process a frame
frame_counter = 0

def nothing(x):
    pass

# Function to capture frames from the camera
def capture_frame(q):
	global frame, frame_counter
	global Session_Paused, Session_Started, Session_Timer, debounce, timelookingatroad, timenotlookingatroad, starttime, Paused_Time, Pause_Timer, Exit
	cap = cv2.VideoCapture(0)  # Open the webcam

	if not cap.isOpened():
		print("Error: Could not open webcam.")
		return
	
	while True:
		if Exit == True:
			sys.exit()
			exit()
		ret, new_frame = cap.read()  # Capture a frame
		if not ret:
			print("Error: Failed to capture image.")
			break

		with frame_lock:  # Lock to safely update the global frame
			frame = new_frame
			
		# Magic	
		if ((Session_Paused == False) and (Session_Started == True)):
			starttime = starttime + Paused_Time
			Session_Timer = (endtime - starttime)
			if Paused_Time != 0:
				Paused_Time = 0
			Pause_Timer = endtime
			#print("Session Start")
		elif ((Session_Paused == True) and (Session_Started == True)):
			Paused_Time = (endtime - Pause_Timer)
			#print("Session Pause")
		elif (Session_Started == False):
			Session_Paused = False
			debounce = False
			timelookingatroad = 0
			timenotlookingatroad = 0
			Session_Timer = 0
			#print("Session End")		
			
		#print(time.time())
		endtime = time.time()
		frame_counter += 1
		time.sleep(0.03)  # Sleep to capture at ~30 fps

	cap.release()

# 800x480
# Function to display the frame with empty space to the right
def display_frame(q):
	global Session_Paused, Session_Started, Session_Timer, debounce, timelookingatroad, timenotlookingatroad, Previous_Time, starttime, timepressed, firsttime, PastSession_Timer, Pasttimelookingatroad, Pasttimenotlookingatroad, ClearTab, Exit
	global frame
	endtime = time.time()
	timeleft = 0
	timeleft2 = 0
	while True:
		with frame_lock:  # Lock to safely access the global frame
			if frame is not None:
				firsttime = time.time()
				
				# Set resolution
				frame = cv2.resize(frame, (480, 480), interpolation=cv2.INTER_AREA)
				
				# Get the original frame dimensions (height, width)
				height, width, _ = frame.shape

				# Create a new frame with extra width for empty space to the right
				new_height = width + 320  # Add offset to the original width for space on the right
				new_frame = np.zeros((new_height, width, 3), dtype=np.uint8)  # Create a black frame (empty space)

				# Place the original frame on the left side of the new frame
				new_frame[:height, :] = frame
				
				# Process data
				data = q.get()
				Looking_at_roadRAW, face_foundRAW = data
				
				if Looking_at_roadRAW == True:
					Looking_at_road = True
					timeleft = 0.5
				elif timeleft >= 0:
					Looking_at_road = True
					
				if timeleft <= 0:
					Looking_at_road = False
				
				if Looking_at_roadRAW == False:
					timeleft = timeleft - (firsttime - endtime)
					
					
				if face_foundRAW == True:
					face_found = True
					timeleft2 = 1
				elif timeleft >= 0:
					face_found = True
					
				if timeleft <= 0:
					face_found = False
				
				if face_foundRAW == False:
					timeleft2 = timeleft2 - (firsttime - endtime)
								
				# Calculate time at road
				if Looking_at_road == True:
					timelookingatroad = timelookingatroad + Session_Timer - Previous_Time
				elif Looking_at_road == False:
					timenotlookingatroad = timenotlookingatroad + Session_Timer - Previous_Time
				Previous_Time = Session_Timer
				
				# Draw rectangle and text
				textsize = 1.1
								
				cv2.rectangle(new_frame, (0, 480), (480, 800), (255, 255, 255), -1)
				
				if face_found == True:
					cv2.putText(new_frame, "Calibrated : True", (0, 18), cv2.FONT_HERSHEY_DUPLEX, 0.75, (0, 150, 0), 2)
				elif face_found == False:
					cv2.putText(new_frame, "Calibrated : False", (0, 18), cv2.FONT_HERSHEY_DUPLEX, 0.75, (0, 0, 0), 2)
				
				cv2.putText(new_frame, "Session time: " + str(round(abs(Session_Timer),1)), (0, 510), cv2.FONT_HERSHEY_DUPLEX, textsize, (0, 0, 0), 2)
				cv2.putText(new_frame, "Time Attending: " + str(round(abs(timelookingatroad),1)), (0, 545), cv2.FONT_HERSHEY_DUPLEX, textsize, (0, 0, 0), 2)
				cv2.putText(new_frame, "Time Unattending: " + str(round(abs(timenotlookingatroad),1)), (0, 580), cv2.FONT_HERSHEY_DUPLEX, textsize, (0, 0, 0), 2)
				if (timelookingatroad or Session_Timer) == 0:
					cv2.putText(new_frame, "Attentiveness Score: 0%", (0, 615), cv2.FONT_HERSHEY_DUPLEX, textsize, (0, 0, 0), 2)
				else:
					cv2.putText(new_frame, "Attentiveness Score: " + str(round(abs(timelookingatroad/Session_Timer*100),1)) + "%", (0, 615), cv2.FONT_HERSHEY_DUPLEX, textsize, (0, 0, 0), 2)
				
				# Button position and size
				x, y, w, h = 240, 625, 240, 175
				x2, y2, w2, h2 = 0, 625, 240, 175
				x3, y3, w3, h3 = 400, 40, 40, 40
				x4, y4, w4, h4 = 0, 440, 120, 40
				
				# Check if button is clicked
				def on_mouse(event, mx, my, flags, param):
					global debounce
					global Session_Paused, Session_Started, Session_Timer, timepressed, starttime, firsttime, ClearTab, Exit
					if event == cv2.EVENT_LBUTTONDOWN:
						if x <= mx <= x + w and y <= my <= y + h:
							print("End pressed")	
							Session_Started = False		
							timepressed = False
					if event == cv2.EVENT_LBUTTONDOWN:
						if x2 <= mx <= x2 + w2 and y2 <= my <= y2 + h2:
							firsttime = False
							if timepressed == False:
								starttime = time.time()
								timepressed = True
							Session_Started = True	
							if debounce == True:
								Session_Paused = True
								debounce = False
								print("paused")
								pass
							elif debounce == False:
								Session_Paused = False
								debounce = True
								print("unpaused")
								pass
					if event == cv2.EVENT_LBUTTONDOWN:
						if x3 <= mx <= x3 + w3 and y3 <= my <= y3 + h3:	
							ClearTab = True
					if event == cv2.EVENT_LBUTTONDOWN:
						if x4 <= mx <= x4 + w4 and y4 <= my <= y4 + h4:	
							Exit = True
							
				if Exit == True:
					sys.exit()
					exit()
		
				# Draw buttons
				if (Session_Paused == False) and (timepressed == True):
					cv2.rectangle(new_frame, (x2, y2), (x2 + w2, y2 + h2), (0, 128, 255), -1)
					cv2.putText(new_frame, "Pause Session", (x2 + 5, y2 + 35), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
				else:
					cv2.rectangle(new_frame, (x2, y2), (x2 + w2, y2 + h2), (0, 200, 0), -1)
					cv2.putText(new_frame, "Start Session", (x2 + 15, y2 + 35), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
				cv2.rectangle(new_frame, (x, y), (x + w, y + h), (0, 0, 255), -1)
				cv2.putText(new_frame, "End Session", (x + 15, y + 35), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
				cv2.rectangle(new_frame, (x4, y4), (x4 + w4, y4 + h4 - 1), (155, 155, 255), -1)
				cv2.putText(new_frame, "EXIT", (x4 + 25, y4 + 31), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 155), 2)
				
				# Give happy screen
				if (timepressed == True):
					PastSession_Timer = Session_Timer
					ClearTab = False
					Pasttimelookingatroad = timelookingatroad
					Pasttimenotlookingatroad = timenotlookingatroad
									
				if (Session_Started == False and firsttime == False) and ClearTab == False:
					cv2.rectangle(new_frame, (40, 40), (440, 240), (255, 255, 255), -1)
					cv2.rectangle(new_frame, (400, 40), (440, 80), (155, 155, 255), -1)
					cv2.putText(new_frame, "X", (410, 70), cv2.FONT_HERSHEY_DUPLEX, textsize/1.25, (0, 0, 155), 2)
					cv2.putText(new_frame, "REPORT:", (40, 85), cv2.FONT_HERSHEY_DUPLEX, textsize/.75, (0, 0, 0), 2)
					cv2.putText(new_frame, "Session time: " + str(round(abs(PastSession_Timer),1)), (40,120), cv2.FONT_HERSHEY_DUPLEX, textsize/1.25, (0, 0, 0), 1)
					cv2.putText(new_frame, "Time Attending: " + str(round(abs(Pasttimelookingatroad),1)), (40, 155), cv2.FONT_HERSHEY_DUPLEX, textsize/1.25, (0, 0, 0), 1)
					cv2.putText(new_frame, "Time Unattending: " + str(round(abs(Pasttimenotlookingatroad),1)), (40, 190), cv2.FONT_HERSHEY_DUPLEX, textsize/1.25, (0, 0, 0), 1)
					cv2.putText(new_frame, "Attentiveness Score: " + str(round(abs(Pasttimelookingatroad/PastSession_Timer*100),1)) + "%", (40, 225), cv2.FONT_HERSHEY_DUPLEX, textsize/1.25, (0, 0, 0), 1)

				# Display the new frame with the empty space on the right that fits the screen
				cv2.namedWindow("Captured Frame", cv2.WINDOW_NORMAL)
				cv2.setMouseCallback("Captured Frame", on_mouse)
				cv2.setWindowProperty("Captured Frame", cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)
				cv2.imshow('Captured Frame', new_frame)
				
				endtime = time.time()

		if cv2.waitKey(1) & 0xFF == ord('q'):
			break  # Press 'q' to quit

	cv2.destroyAllWindows()

# Function to process a frame (e.g., every second)
def process_frame(q):
	global Looking_at_roadP
	global xupperlimit
	global xlowerlimit
	global yupperlimit
	global ylowerlimit
	global starttime
	global timewatchingroad
	global timenotwatchingroad
	global elapsedtime
	global face_foundP
	global frame
	global frame_counter
	global Exit
	gaze = GazeTracking()
	while True:
		if Exit == True:
			sys.exit()
			exit()
		try: 
			if frame is not None:
				# Refresh gaze tracking with the current frame
				gaze.refresh(frame)
				
				# Process data
				previoustime = elapsedtime
				endtime = time.time()
				elapsedtime = endtime - starttime
				frametime = elapsedtime - previoustime
				h_ratio = gaze.horizontal_ratio()
				v_ratio = gaze.vertical_ratio()	
			
				if (h_ratio == None or v_ratio == None):
					Looking_at_roadP = False 
					face_foundP = False
				elif (xlowerlimit <= h_ratio <= xupperlimit) and (ylowerlimit <= v_ratio <= yupperlimit):
					Looking_at_roadP = True
					face_foundP = True
				else:
					Looking_at_roadP = False
				
				if Looking_at_roadP == True:
					timewatchingroad = timewatchingroad + frametime
				elif face_foundP == True:
					timenotwatchingroad = timenotwatchingroad + frametime
				
				q.put((Looking_at_roadP, face_foundP))
			
				# Developer Data
				#print("Gaze Position: ", h_ratio, v_ratio)
				#print("Looking at road? ", Looking_at_road)
			
				# Reset counter after processing
				frame_counter = 0
		except:
			print("......................An Error has Occured Processing the Frame......................")
				

# Create threads for capturing, displaying, and processing
message_queue = queue.Queue()
capture_thread = threading.Thread(target=capture_frame, args=(message_queue,))
display_thread = threading.Thread(target=display_frame, args=(message_queue,))
process_thread = threading.Thread(target=process_frame, args=(message_queue,))

# Start the threads
capture_thread.start()
display_thread.start()
process_thread.start()

# Wait for the threads to finish (in this case, they run forever, so use a timeout or condition to stop)
capture_thread.join()
display_thread.join()
process_thread.join()

