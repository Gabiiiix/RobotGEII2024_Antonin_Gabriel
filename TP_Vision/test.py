

import cv2
import numpy as np
from urllib.request import urlopen
from math import sqrt

req = urlopen('http://www.vgies.com/downloads/robocup.png')
arr = np.asarray(bytearray(req.read()), dtype = np.uint8)
img = cv2.imdecode(arr , -1)

B, G, R = cv2.split(img)
#cv2.imshow("original", img)
#cv2.waitKey(0)

#cv2.imshow("blue", B)
#cv2.waitKey(0)

#cv2.imshow("Green", G)
#cv2.waitKey(0)

#cv2.imshow("Red", R)
#cv2.waitKey(0)


imagehsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
H, S, V = cv2.split(imagehsv)
#cv2.imshow("Hue", H)
#cv2.waitKey(0)
#cv2.imshow("Saturation", S)
#cv2.waitKey(0)
#cv2.imshow("Value", V)
#cv2.waitKey(0)


yellow_uniform = np.zeros_like(img)
blue_uniform = np.zeros_like(img)
green_uniform = np.zeros_like(img)

yellow_uniform[:] = (0,255,255)
green_uniform[:] = (0,255,0)
blue_uniform[:] = (255,0,0)

#Definition des limites basses et hautes de la couleur jaune en HSV
#A noter que le jaune se situe vers les 25 degres dans la roue de couleur HSV en H
lower_yellow = np.array([20, 100, 100])
upper_yellow = np.array([30,255,255])
#Masquage de l’image HSV pour ne garder que les zones jaunes
imagemaskyellow = cv2.inRange(imagehsv, lower_yellow, upper_yellow)
yellow_color = cv2.bitwise_and(yellow_uniform, yellow_uniform, mask=imagemaskyellow)
#cv2.imshow("Image Masque Jaune", yellow_color)
#cv2.waitKey(0)


lower_green = np.array([40, 50, 50])
upper_green = np.array([80, 255 ,255])

imagemaskgreen = cv2.inRange(imagehsv, lower_green, upper_green)
green_color = cv2.bitwise_and(green_uniform,green_uniform,mask=imagemaskgreen)
#cv2.imshow("Image Masque Vert", green_color)
#cv2.waitKey(0)

lower_blue = np.array([90, 10, 10])
upper_blue = np.array([150, 120, 120])

imagemaskblue = cv2.inRange(imagehsv, lower_blue, upper_blue)
blue_color = cv2.bitwise_and(blue_uniform,blue_uniform, mask=imagemaskblue)
#cv2.imshow("Image Masque Bleu", blue_color)
#cv2.waitKey(0)

image_GY = cv2.add(green_color, yellow_color)
image_final = cv2.add(blue_color, image_GY)

#cv2.imshow("Image Combinee", image_final)


#TRANSFORMATIONS MANUELLES D’UNE IMAGE
height = img.shape[0]
width = img.shape[1]
channels = img.shape[2]
imgTransform = img
x_centre = width / 2
y_centre = height / 2
for x in range(0, (int)(width)):
    for y in range (0, (int)(height)):
        x_rel = x - x_centre
        y_rel = y - y_centre
        valeur = sqrt(x_rel*x_rel + y_rel*y_rel)
        if(valeur <= 50):
            imgTransform[y,x][0] *= 1
            imgTransform[y,x][1] *= 1
            imgTransform[y,x][2] *= 1
        else:
            imgTransform[y,x][0] *= 1
            imgTransform[y,x][1] *= 1
            imgTransform[y,x][2] *= 1
#cv2.imshow("Transformation␣manuelle␣de␣l’image", imgTransform)

imageTransformhsv = cv2.cvtColor(imgTransform, cv2.COLOR_BGR2HSV)

lower_green = np.array([40, 50, 50])
upper_green = np.array([80, 255 ,255])

imagemaskgreen = cv2.inRange(imageTransformhsv, lower_green, upper_green)
green_color = cv2.bitwise_and(green_uniform,green_uniform,mask=imagemaskgreen)
#cv2.imshow("Image Masque Vert", green_color)
#cv2.waitKey(0)

# importing library for plotting
from matplotlib import pyplot as plt
#Conversion de l’image en niveaux de gris
imageGray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
#cv2.imshow('Grayscale', imageGray)
#Calcul de l’histogramme
hist,bins = np.histogram(imageGray.flatten(),256,[0,256])
#Calcul de l’histogramme cumule
cdf = hist.cumsum()
cdf_normalized = cdf * float(hist.max()) / cdf.max()
#Affichage de l’histogramme cumule en bleu
plt.plot(cdf_normalized, color = 'b')
#Affichage de l’histogramme en rouge
plt.hist(imageGray.flatten(),256,[0,256], color = 'r')
plt.xlim([0,256])
plt.legend(('cdf','histogram'), loc = 'upper left')

imgEqu = cv2.equalizeHist(imageGray)
histEq,binsEq = np.histogram(imgEqu.flatten(),256,[0,256])
cdfEq = histEq.cumsum()
cdfEq_normalized = cdfEq * float(histEq.max()) / cdfEq.max()
#cv2.imshow('Image égalise', imgEqu)
plt.clf()
plt.plot(cdfEq_normalized, color = 'b')
plt.hist(imgEqu.flatten(),256,[0,256], color = 'r')
plt.xlim([0,256])
plt.legend(('cdfEq','histogramEq'), loc = 'upper left')
#plt.show()


Heq = cv2.equalizeHist(H)
Seq = cv2.equalizeHist(S)
Veq = cv2.equalizeHist(V)


#cv2.imshow('H equalized', Heq)
#cv2.imshow('S equalized', Seq)
#cv2.imshow('V equalized', Veq)

hsv_reformed = cv2.merge([H, S, Veq])
img_reformed = cv2.cvtColor(hsv_reformed, cv2.COLOR_HSV2BGR)
#cv2.imshow("Image reformée", img_reformed)
#cv2.waitKey(0)

#FILTRAGE LINEAIRE

img_blur = cv2.GaussianBlur(img,(7,7),0)
cv2.imshow('image flou', img_blur)

sobelx = cv2.Sobel(src=img_blur, ddepth=cv2.CV_64F, dx=1, dy=0, ksize=5) 
sobely = cv2.Sobel(src=img_blur, ddepth=cv2.CV_64F, dx=0, dy=1, ksize=5) 
sobelxy = cv2.Sobel(src=img_blur, ddepth=cv2.CV_64F, dx=1, dy=1, ksize=5)

# Display Sobel Edge Detection Images
cv2.imshow('Sobel X', sobelx)
cv2.waitKey(0)
cv2.imshow('Sobel Y', sobely)
cv2.waitKey(0)
cv2.imshow('Sobel X Y using Sobel() function', sobelxy)
cv2.waitKey(0)


# Filtrage a l’aide de noyaux de convolution definis manuellement
kernel1 = np.array([[-1, -2, -1],
[0, 0, 0],
[1, 2, 1]])
imgSobelHSup = cv2.filter2D(src=img_blur, ddepth=-1, kernel=kernel1)
cv2.imshow('Sobel a la main wesh',imgSobelHSup)
