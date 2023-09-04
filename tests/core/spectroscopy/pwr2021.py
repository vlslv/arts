import pyarts
import numpy as np

f = np.linspace(1e9, 1000e9, 101)
x_h2o = 5e-3
x_o2 = 0.21
x_n2 = 0.781

x = pyarts.arts.predef.get_h2o_pwr2021(f, 1e4, 250, x_h2o)
y = pyarts.arts.predef.get_o2_pwr2021(f, 1e4, 250, x_o2, x_h2o)
z = pyarts.arts.predef.get_n2_pwr2021(f, 1e4, 250, x_n2, x_h2o)

x_ref = np.array([8.62520957e-11, 1.27448533e-08, 1.09635056e-06, 1.19335086e-07,
                  1.38755048e-07, 1.97312377e-07, 2.74382912e-07, 3.67716809e-07,
                  4.77172708e-07, 6.03324733e-07, 7.47361291e-07, 9.11405841e-07,
                  1.09939698e-06, 1.31934748e-06, 1.58987332e-06, 1.96315113e-06,
                  2.63523672e-06, 4.89622033e-06, 7.91407608e-05, 1.21911857e-05,
                  4.95950237e-06, 4.23066987e-06, 4.24652254e-06, 4.48408312e-06,
                  4.82888705e-06, 5.24740215e-06, 5.73173017e-06, 6.28742356e-06,
                  6.93475559e-06, 7.72580534e-06, 8.82381000e-06, 1.10774252e-05,
                  4.57196863e-05, 2.71502878e-05, 1.53641131e-05, 1.72206748e-05,
                  2.43566356e-05, 6.24987290e-05, 1.59990728e-02, 6.27709570e-05,
                  3.27656890e-05, 2.96366971e-05, 3.26561478e-05, 4.46512073e-05,
                  2.00165804e-04, 6.78209354e-04, 7.50634668e-05, 4.59940707e-04,
                  8.45227332e-05, 1.09718005e-04, 1.19075919e-04, 1.71008861e-04,
                  2.74508548e-04, 5.20967895e-04, 1.35823217e-03, 8.90814097e-03,
                  3.19023369e-02, 2.25589145e-03, 7.89396482e-04, 4.17937035e-04,
                  2.72484184e-04, 2.14117670e-04, 1.19850102e-02, 1.52772657e-04,
                  1.19631207e-04, 1.08255564e-04, 1.11987976e-04, 1.02150780e-04,
                  1.06533170e-04, 1.17739861e-04, 1.39854826e-04, 1.83399761e-04,
                  2.78575844e-04, 5.40295978e-04, 1.75276877e-03, 7.19969613e-02,
                  3.76276519e-03, 8.27247370e-04, 3.84187488e-04, 2.38426044e-04,
                  1.72837507e-04, 1.37820420e-04, 1.17036853e-04, 1.03824951e-04,
                  9.50637798e-05, 8.91575449e-05, 8.52748713e-05, 8.30751063e-05,
                  8.27684503e-05, 8.59631506e-05, 1.01667307e-04, 2.65757879e-04,
                  5.46475867e-04, 1.11548324e-04, 8.63142461e-05, 7.98505819e-05,
                  7.74310085e-05, 7.64433009e-05, 7.61223007e-05, 7.61730553e-05,
                  7.64593697e-05])


y_ref = np.array([2.35915937e-08, 2.76152740e-08, 4.08711952e-08, 7.61583843e-08,
                  1.94360376e-07, 1.34233864e-06, 3.69992915e-04, 8.10346520e-07,
                  2.28236500e-07, 1.27442393e-07, 1.17625792e-07, 3.09873425e-07,
                  3.39614342e-06, 1.20242381e-07, 4.53228300e-08, 2.67324530e-08,
                  1.87223528e-08, 1.42762160e-08, 1.14449645e-08, 9.49468012e-09,
                  8.10181113e-09, 7.15039294e-09, 6.92662718e-09, 2.19932096e-08,
                  8.79300105e-09, 5.15388582e-09, 4.45413881e-09, 4.13016363e-09,
                  3.97881714e-09, 3.96840702e-09, 4.11262846e-09, 4.46106447e-09,
                  5.12408235e-09, 6.36007557e-09, 8.91018210e-09, 1.59169704e-08,
                  6.03223059e-08, 7.53495586e-07, 4.15115272e-08, 3.68560855e-08,
                  1.13688176e-07, 1.76518951e-07, 2.05593939e-06, 1.12208359e-06,
                  1.65471676e-07, 7.31792330e-08, 5.40781067e-08, 7.35783321e-08,
                  3.43860844e-07, 1.46308166e-06, 1.01608012e-07, 3.92198464e-08,
                  2.27846500e-08, 1.58476514e-08, 1.22066918e-08, 1.01695222e-08,
                  1.05179167e-08, 1.47232015e-08, 7.24910435e-09, 6.45658773e-09,
                  6.09957657e-09, 5.95596946e-09, 5.98976071e-09, 6.20884876e-09,
                  6.65256471e-09, 7.40380140e-09, 8.62737867e-09, 1.06748570e-08,
                  1.44179063e-08, 2.26457324e-08, 4.96140310e-08, 3.58318497e-07,
                  4.06044220e-07, 1.97526136e-07, 6.72643465e-08, 1.10102306e-07,
                  3.12482094e-07, 4.41622739e-06, 1.45920895e-06, 2.37621868e-07,
                  1.06540579e-07, 8.03358356e-08, 1.22431238e-07, 1.15909330e-06,
                  5.26173267e-07, 8.72216706e-08, 3.96670341e-08, 2.46192692e-08,
                  1.78832758e-08, 1.79399465e-08, 1.53510127e-08, 9.55908599e-09,
                  7.92935105e-09, 6.84564145e-09, 6.02728416e-09, 5.37885932e-09,
                  4.85034120e-09, 4.41076372e-09, 4.03933530e-09, 3.72139341e-09,
                  3.44623781e-09])

z_ref = np.array([1.77186970e-12, 2.13943345e-10, 7.79063024e-10, 1.69547109e-09,
                  2.96049808e-09, 4.57050366e-09, 6.52092877e-09, 8.80635979e-09,
                  1.14206031e-08, 1.43567680e-08, 1.76073565e-08, 2.11643569e-08,
                  2.50193405e-08, 2.91635579e-08, 3.35880346e-08, 3.82836629e-08,
                  4.32412893e-08, 4.84517965e-08, 5.39061784e-08, 5.95956072e-08,
                  6.55114936e-08, 7.16455379e-08, 7.79897738e-08, 8.45366033e-08,
                  9.12788247e-08, 9.82096530e-08, 1.05322734e-07, 1.12612151e-07,
                  1.20072430e-07, 1.27698530e-07, 1.35485846e-07, 1.43430191e-07,
                  1.51527784e-07, 1.59775239e-07, 1.68169541e-07, 1.76708031e-07,
                  1.85388386e-07, 1.94208596e-07, 2.03166948e-07, 2.12261999e-07,
                  2.21492562e-07, 2.30857683e-07, 2.40356620e-07, 2.49988828e-07,
                  2.59753940e-07, 2.69651750e-07, 2.79682196e-07, 2.89845346e-07,
                  3.00141385e-07, 3.10570602e-07, 3.21133374e-07, 3.31830161e-07,
                  3.42661491e-07, 3.53627952e-07, 3.64730184e-07, 3.75968870e-07,
                  3.87344732e-07, 3.98858517e-07, 4.10511001e-07, 4.22302976e-07,
                  4.34235249e-07, 4.46308638e-07, 4.58523965e-07, 4.70882055e-07,
                  4.83383735e-07, 4.96029827e-07, 5.08821148e-07, 5.21758508e-07,
                  5.34842710e-07, 5.48074545e-07, 5.61454792e-07, 5.74984217e-07,
                  5.88663575e-07, 6.02493604e-07, 6.16475029e-07, 6.30608557e-07,
                  6.44894882e-07, 6.59334680e-07, 6.73928613e-07, 6.88677324e-07,
                  7.03581442e-07, 7.18641578e-07, 7.33858328e-07, 7.49232270e-07,
                  7.64763970e-07, 7.80453973e-07, 7.96302813e-07, 8.12311007e-07,
                  8.28479056e-07, 8.44807449e-07, 8.61296657e-07, 8.77947141e-07,
                  8.94759346e-07, 9.11733703e-07, 9.28870631e-07, 9.46170538e-07,
                  9.63633815e-07, 9.81260846e-07, 9.99051999e-07, 1.01700763e-06,
                  1.03512810e-06])

assert np.allclose(x, x_ref), "Error in H2O-PWR2021"
assert np.allclose(y, y_ref), "Error in O2-PWR2021"
assert np.allclose(z, z_ref), "Error in N2-PWR2021"