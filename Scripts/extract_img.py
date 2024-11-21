import pyrealsense2 as rs
import cv2
import numpy as np
import os

video_path=".bag/file/captured/by/realsense"

pipeline = rs.pipeline()
config = rs.config()

pose_id=0
cnt=set()

def extract_pose(file):
    global pose_id

    pose_dir=video_path+str(pose_id)
    pose_id+=1
    if not os.path.exists(pose_dir):
        os.mkdir(pose_dir)
    
    config.enable_device_from_file(file)  
    pipeline.start(config)

    i=0
    while True:
        frames = pipeline.wait_for_frames()

        color_frame = frames.get_color_frame()
        id=frames.get_frame_number()
        if id in cnt:
            break
        cnt.add(id)

        color_image = np.asanyarray(color_frame.get_data())
        cv2.imwrite(f'{pose_dir}/{i}.png', color_image)
        i+=1
    pipeline.stop()

for root, dirs, files in os.walk(video_path):
    for file in files:
        print(f'extract:{file}')
        extract_pose(os.path.join(root,file))
