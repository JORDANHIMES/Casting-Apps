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
#Z = np.concatenate((L,C,R))

where_are_NaNs = np.isnan(x)
x[where_are_NaNs] = 0
where_are_NaNs = np.isnan(y)
y[where_are_NaNs] = 0
where_are_NaNs = np.isnan(Z)
Z[where_are_NaNs] = 0

xi = np.linspace(np.min(x), np.max(x),8000)
yi = np.linspace(np.min(y), np.max(y),1500)

XI, YI = np.meshgrid(xi, yi)
points = np.vstack((x,y)).T
values = np.asarray(Z)
points = np.asarray(points)

z=Z

z_min, z_max = -np.abs(z).max(), np.abs(z).max()


fig, axs = plt.subplots(2, 2)

ax = axs[0, 0]
c = ax.pcolor(x, y, z, cmap='RdBu', vmin=z_min, vmax=z_max)
ax.set_title('pcolor')
# set the limits of the plot to the limits of the data
ax.axis([x.min(), x.max(), y.min(), y.max()])
fig.colorbar(c, ax=ax)

ax = axs[0, 1]
c = ax.pcolormesh(x, y, z, cmap='RdBu', vmin=z_min, vmax=z_max)
ax.set_title('pcolormesh')
# set the limits of the plot to the limits of the data
ax.axis([x.min(), x.max(), y.min(), y.max()])
fig.colorbar(c, ax=ax)

ax = axs[1, 0]
c = ax.imshow(z, cmap='RdBu', vmin=z_min, vmax=z_max,
              extent=[x.min(), x.max(), y.min(), y.max()],
              interpolation='nearest', origin='lower')
ax.set_title('image (nearest)')
fig.colorbar(c, ax=ax)

ax = axs[1, 1]
c = ax.pcolorfast(x, y, z, cmap='RdBu', vmin=z_min, vmax=z_max)
ax.set_title('pcolorfast')
fig.colorbar(c, ax=ax)

fig.tight_layout()
plt.show()