import numpy as np
from scipy import interpolate
from scipy.signal import filtfilt, butter
import matplotlib as mpl
import matplotlib.pyplot as plt
from matplotlib import cm
import argparse
import time


#CLI ARGUMENTS
ap = argparse.ArgumentParser()
ap.add_argument("-f", "--file", required =True, help = "filename including extension, ex. 'thickness.csv'")
ap.add_argument("-t","--title",required=False,help="title in plot, ex. ''LOT1 REEDS 11-14-2018''")
ap.add_argument("-s","--save", action='store_true', dest = 'savetrue',required=False,help="save the plot to current directory with the given file name,''LOT1-REEDS-11-14-2018.jpg''")
args = vars(ap.parse_args())

windowcount = 1
signal = np.genfromtxt(args["file"],delimiter=',',dtype=float)
time = np.genfromtxt(args["file"],delimiter=',',dtype = str)
timestamp=time[:,0]
print(timestamp)
t = signal[:,1]
L = signal[:,2]
C = signal[:,3]
R = signal[:,4]
t = t*20.32

b, a = butter(2,0.05)

#Lfilt = filtfilt(b,a,L)
Lpos = np.tile(-660,len(L))
#Cfilt = filtfilt(b,a,C)
Cpos = np.tile(0,len(C))
#Rfilt = filtfilt(b,a,R)
Rpos = np.tile(660,len(R))

while (windowcount < (len(t)-94)) :
	
	windowsize = 95
	windowlow = windowcount-1
	windowhigh = windowcount+windowsize-1
	tstampwindow = timestamp[windowlow:windowhigh]
	tstamp= np.concatenate((tstampwindow,tstampwindow,tstampwindow))
	x = np.concatenate((t[windowlow:windowhigh],t[windowlow:windowhigh],t[windowlow:windowhigh]))
	
	z1 = L[windowlow:windowhigh]
	z2 = C[windowlow:windowhigh]
	z3 = R[windowlow:windowhigh]

	y = np.concatenate((Lpos[windowlow:windowhigh],Cpos[windowlow:windowhigh],Rpos[windowlow:windowhigh]))
	#y = np.concatenate(y1,y2,y3)
	
	Lfilt = filtfilt(b,a,z1)
	Cfilt = filtfilt(b,a,z2)
	Rfilt = filtfilt(b,a,z3)
	Z = np.concatenate((Lfilt,Cfilt,Rfilt))
	#Z = np.concatenate((Lfilt[windowlow:windowhigh],Cfilt[windowlow:windowhigh],Rfilt[windowlow:windowhigh]))
	#Zunfilt = np.concatenate((L,C,R))

	where_are_NaNs = np.isnan(x)
	x[where_are_NaNs] = np.nanmin(x)
	where_are_NaNs = np.isnan(y)
	y[where_are_NaNs] = 0
	where_are_NaNs = np.isnan(Z)
	Z[where_are_NaNs] = 0

	xi = np.linspace(np.min(x), np.max(x),3*95)
	yi = np.linspace(np.min(y), np.max(y),95)

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

	aspectratio = (660*2)/(95*20.32)
	levels = np.arange(-3,3,.25)


	cmap = cm.bwr
	#norm = mpl.colors.Normalize(vmin = -2, vmax = 2, clip = False)
	CS= plt.contour(YI,XI,Z1,15,linewidths=1,colors='k')
	#CS= plt.contourf(YI,XI,Z1,15,cmap=cmap, vmin=-5, vmax = 5)
	CS= plt.contourf(YI,XI,Z1,15,cmap=cmap,levels=levels)
	#plt.scatter(y,x,)
	#plt.clim(-2,2)
	cbar = plt.colorbar(CS, extend = 'both',ticks=np.arange(-3,3,.25))
	#cbar.set_ticks([-2,0,2])
	#cbar.minorticks_on()
	plt.title(str(tstampwindow[1]))
	
	plt.axes().set_aspect(aspectratio)
	#ax = plt.gca()
	

	ax2 = plt.axes().twinx()
	ax2.plot(range(95),tstampwindow)
	

	#ax.tight_layout()
	plt.ion()
	plt.show()

	windowcount = windowcount+1
	#wait = input("press any key")

	#plt.savefig(args["title"]+"-" + str(windowcount) + ".png")
	print("Saved plot")

	plt.pause(.25)
	plt.cla()
	plt.clf()