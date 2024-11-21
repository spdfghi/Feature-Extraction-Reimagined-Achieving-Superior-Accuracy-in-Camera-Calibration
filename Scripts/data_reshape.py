import cv2
import pupil_apriltags as apriltag
import os

path='D:\\CalibPaper\\data\\image_last\\'
if not os.path.exists(path+"star16"):
    os.mkdir(path+"star16")
for i in range(8):
    if not os.path.exists(path+"star8_"+str(i+1)):
        os.mkdir(path+"star8_"+str(i+1))

detector = apriltag.Detector()
for i in range(38):
    dir=path+str(i)
    for root,dirs,files in os.walk(dir):
        for file in files:
            image=cv2.imread(os.path.join(root,file),0)
            tags = detector.detect(image)

            for tag in tags:
                id=tag.tag_id
                if id==8:
                    dst=path+"star16"
                else:
                    dst=path+"star8_"+str(id+1)
                dst+="\\"+str(i)+".png"

                cv2.imwrite(dst,image)
        