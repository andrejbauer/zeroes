import cv2
import numpy as np
import mediapipe as mp
import matplotlib.pyplot as plt

# See also https://community.wolfram.com/groups/-/m/t/2048213

# Initialize Mediapipe Pose class.
mp_pose = mp.solutions.pose
pose = mp_pose.Pose()

# Initialize webcam.
cap = cv2.VideoCapture(0)

# Function to normalize coordinates
def normalize_coordinates(coords, width, height):
    return [(x / width, y / height) for x, y in coords]

# Set up the interactive plot
plt.ion()
fig, ax = plt.subplots()

# Function to update plot
def update_plot(roots):
    ax.clear()
    ax.plot(np.real(roots), np.imag(roots), 'bo')  # Plot roots as blue disks
    ax.set_xlabel('Real Part')
    ax.set_ylabel('Imaginary Part')
    ax.set_title('Polynomial Roots')
    ax.set_aspect('equal')
    ax.grid(True)
    plt.draw()
    plt.pause(0.001)

while True:
    ret, frame = cap.read()
    if not ret:
        break
    
    # Convert the frame to RGB as Mediapipe expects RGB input.
    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    
    # Perform pose detection.
    results = pose.process(frame_rgb)
    
    # Extract pose landmarks.
    if results.pose_landmarks:
        height, width, _ = frame.shape
        landmarks = results.pose_landmarks.landmark
        key_points = [(int(lm.x * width), int(lm.y * height)) for lm in landmarks]

        # Normalize coordinates.
        normalized_key_points = normalize_coordinates(key_points, width, height)

        # Flatten the list of tuples.
        parameters = np.array([float(10 * coord) for point in normalized_key_points for coord in point])

        # Make it a monic polynomial
        parameters[0] = 1

        print(parameters)

        # Use the parameters as coefficients of a polynomial and find its roots
        roots = np.roots(parameters)

        # Update the plot with the new roots
        update_plot(roots)

    # Display the frame with landmarks.
    annotated_image = frame.copy()
    mp.solutions.drawing_utils.draw_landmarks(annotated_image, results.pose_landmarks, mp_pose.POSE_CONNECTIONS)
    cv2.imshow('Pose Detection', annotated_image)
    
    if cv2.waitKey(5) & 0xFF == 27:
        break

cap.release()
cv2.destroyAllWindows()
plt.ioff()
plt.show()
