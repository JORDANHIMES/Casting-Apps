import numpy as np
from scipy import interpolate
from scipy.signal import filtfilt, butter
import matplotlib.pyplot as plt
from matplotlib import cm


signal = np.genfromtxt('thickness.csv',delimiter=',')
t = signal[:,0]
L = signal[:,1]
C = signal[:,2]
R = signal[:,3]

t=1.7*t
b, a = butter(2,0.05)

Lfilt = filtfilt(b,a,L)
Lpos = np.tile(-762,len(L))
Cfilt = filtfilt(b,a,C)
Cpos = np.tile(0,len(C))
Rfilt = filtfilt(b,a,R)
Rpos = np.tile(762,len(R))

x = np.concatenate((t,t,t))	

y = np.concatenate((Lpos,Cpos,Rpos))

Z = np.concatenate((Lfilt,Cfilt,Rfilt))
Zunfilt = np.concatenate((L,C,R))

where_are_NaNs = np.isnan(x)
x[where_are_NaNs] = 0
where_are_NaNs = np.isnan(y)
y[where_are_NaNs] = 0
where_are_NaNs = np.isnan(Z)
Z[where_are_NaNs] = 0
where_are_NaNs = np.isnan(Zunfilt)
Z[where_are_NaNs] = 0


xi = np.linspace(np.min(x), np.max(x),8000)
yi = np.linspace(np.min(y), np.max(y),1500)

XI, YI = np.meshgrid(xi, yi)
points = np.vstack((x,y)).T
values = np.asarray(Z)
valuesunfilt = np.asarray(Zunfilt)
points = np.asarray(points)
Z1 = interpolate.griddata(points, values, (XI,YI), method='nearest')
Z2 = interpolate.griddata(points, values, (XI,YI), method='linear')
Z3 = interpolate.griddata(points, values, (XI,YI), method='cubic')
Z4 = interpolate.griddata(points, valuesunfilt, (XI,YI), method='nearest')
Z5 = interpolate.griddata(points, valuesunfilt, (XI,YI), method='linear')
Z6 = interpolate.griddata(points, valuesunfilt, (XI,YI), method='cubic')

aspectratio = (len(t))/(762*2)

plt.subplot(241)
plt.imshow(Z1, origin='lower')
#levels = np.arange(-5,5,1)
CS = plt.contour(xi,yi,Z1,15,linewidths=0.5,colors='k')
CS = plt.contourf(xi,yi,Z1,15,cmap=plt.cm.bwr)
plt.title('Filtered-nearest')
plt.axes().set_aspect(aspectratio)
#plt.colorbar()

plt.subplot(242)
plt.imshow(Z2, origin='lower')
#levels = np.arange(-5,5,1)
CS = plt.contour(xi,yi,Z2,15,linewidths=0.5,colors='k')
CS = plt.contourf(xi,yi,Z2,15,cmap=plt.cm.bwr)
plt.title('Filtered-linear')
plt.axes().set_aspect(aspectratio)
#plt.colorbar()

plt.subplot(243)
plt.imshow(Z3, origin='lower')
#levels = np.arange(-5,5,1)
CS = plt.contour(xi,yi,Z3,15,linewidths=0.5,colors='k')
CS = plt.contourf(xi,yi,Z3,15,cmap=plt.cm.bwr)
plt.title('Filtered-cubic')
plt.axes().set_aspect(aspectratio)
#plt.colorbar()

plt.subplot(245)
plt.imshow(Z4, origin='lower')
#levels = np.arange(-5,5,1)
CS = plt.contour(xi,yi,Z4,15,linewidths=0.5,colors='k')
CS = plt.contourf(xi,yi,Z4,15,cmap=plt.cm.bwr)
plt.title('Unfiltered-nearest')
plt.axes().set_aspect(aspectratio)
#plt.colorbar()

plt.subplot(246)
plt.imshow(Z5, origin='lower')
#levels = np.arange(-5,5,1)
CS = plt.contour(xi,yi,Z5,15,linewidths=0.5,colors='k')
CS = plt.contourf(xi,yi,Z5,15,cmap=plt.cm.bwr)
plt.title('Unfiltered-linear')
plt.axes().set_aspect(aspectratio)
#plt.colorbar()

plt.subplot(247)
plt.imshow(Z6, origin='lower')
#levels = np.arange(-5,5,1)
CS = plt.contour(xi,yi,Z6,15,linewidths=0.5,colors='k')
CS = plt.contourf(xi,yi,Z6,15,cmap=plt.cm.bwr)
plt.title('Unfiltered-cubic')
plt.axes().set_aspect(aspectratio)
#plt.colorbar()

plt.tight_layout()

#plt.imshow(CS,origin='lower')

plt.show()