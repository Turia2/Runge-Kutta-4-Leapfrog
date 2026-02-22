import pandas as pd
import matplotlib.pyplot as plt
import os

carpeta = os.path.dirname(os.path.abspath(__file__))
ruta_rk4 = os.path.join(carpeta, "rk4.csv")
ruta_lf = os.path.join(carpeta, "leapfrog.csv")

df_rk4 = pd.read_csv(ruta_rk4)
df_lf = pd.read_csv(ruta_lf)
print("--- Datos de RK4 ---")
print(df_rk4.head())
print("--- Datos de Leapfrog ---")
print(df_lf.head())

fig, (ax1, ax2) = plt.subplots(1, 2, figsize = (14, 6))

#orbita en el espacio
ax1.plot(df_rk4['X'], df_rk4['Y'], label='Runge-Kutta 4', color='red', alpha=0.7)
ax1.plot(df_lf['X'], df_lf['Y'], label='Leapfrog', color = 'blue', linestyle = '--', alpha = 0.7)
ax1.set_title("Trayectoria")
ax1.set_xlabel("X")
ax1.set_ylabel("Y")
ax1.legend()
ax1.grid(True)
ax1.axis('equal')


#conservación de la energía
ax2.plot(df_rk4['Tiempo'], df_rk4['Error'], label='Error RK4', color='red')
ax2.plot(df_lf['Tiempo'], df_lf['Error'], label='Error Leapfrog', color='blue')

ax2.set_title("Error relativo de la energía total")
ax2.set_xlabel("Tiempo")
ax2.set_ylabel("Error relativo")
ax2.set_yscale('log') 
ax2.legend()
ax2.grid(True, which="both", ls="--", alpha=0.5)

plt.tight_layout()
plt.show()