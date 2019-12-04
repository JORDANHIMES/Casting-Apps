import numpy as np
from scipy import interpolate
from scipy.signal import filtfilt, butter
import matplotlib.pyplot as plt
from matplotlib import cm


signal = np.genfromtxt('thickness.csv',delimiter=',')
#t = signal[1605:2675,0]
#L = signal[1605:2675,1]
#C = signal[1605:2675,2]
#R = signal[1605:2675,3]
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

#print(np.nanmin(x))

where_are_NaNs = np.isnan(x)
x[where_are_NaNs] = np.nanmin(x)
where_are_NaNs = np.isnan(y)
y[where_are_NaNs] = 0
where_are_NaNs = np.isnan(Z)
Z[where_are_NaNs] = 0
#where_are_NaNs = np.isnan(Zunfilt)
#Z[where_are_NaNs] = 0


xi = np.linspace(np.min(x), np.max(x),1500)
yi = np.linspace(np.min(y), np.max(y),10)

XI, YI = np.meshgrid(xi, yi)
points = np.vstack((x,y)).T
values = np.asarray(Z)
#valuesunfilt = np.asarray(Zunfilt)
points = np.asarray(points)
Z1 = interpolate.griddata(points, values, (XI,YI), method='cubic')
#Z2 = interpolate.griddata(points, valuesunfilt, (XI,YI), method='cubic')
#Z3 = interpolate.griddata(points, values, (XI,YI), method='cubic')
#Z4 = interpolate.griddata(points, valuesunfilt, (XI,YI), method='nearest')
#Z5 = interpolate.griddata(points, valuesunfilt, (XI,YI), method='linear')
#Z6 = interpolate.griddata(points, valuesunfilt, (XI,YI), method='cubic')

aspectratio = (len(t))/(762*2)
levels = np.arange(-5,5,1)


#fig, (ax0, ax1) = plt.subplots(nrows=2)
#im = ax0.pcolormesh(points[:,0],points[:,1], Z1, cmap='bwr')
#fig.colorbar(im, ax=ax0)
#ax0.set_title('pcolormesh with levels')
# contours are *point* based plots, so convert our bound into point
# centers
#cf = ax1.contourf(x,y,Z1, levels=levels,cmap='bwr')
#fig.colorbar(cf, ax=ax1)
#ax1.set_title('contourf with levels')
# adjust spacing between subplots so `ax1` title and `ax0` tick labels
# don't overlap
#fig.tight_layout()


plt.contour(xi,yi,Z1,15,linewidths=0.5,colors='k')
plt.contourf(xi,yi,Z1,15,cmap=plt.cm.bwr)

#plt.clim(-3,3)
#plt.draw_all()
plt.colorbar()
plt.title('LOT1')
plt.axes().set_aspect(aspectratio)
plt.tight_layout()

plt.show()