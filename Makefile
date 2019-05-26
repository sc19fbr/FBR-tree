INCLUDES = -I/home/dicl/R-tree/solar_data/hdf5-1.10.5/hdf5/include -I/home/dicl/FBR-tree/solar_data/include
LDFLAGS = -Wl,-rpath,/home/dicl/R-tree/solar_data/hdf5-1.10.5/hdf5/lib -L/home/dicl/R-tree/solar_data/hdf5-1.10.5/hdf5/lib
LIBS = -lhdf5     

all :

breakdown:
	g++ -g -std=c++17 -O3 -o break-cow-ns1-bitlog src/main.cpp -lpthread -DBREAKDOWN 
	g++ -g -std=c++17 -O3 -o break-cow-ns2-bitlog src/main.cpp -lpthread -DBREAKDOWN -DMULTIMETA -DNS2 
	g++ -g -std=c++17 -O3 -o break-cow-ns3-bitlog src/main.cpp -lpthread -DBREAKDOWN -DMULTIMETA -DNS3 
	g++ -g -std=c++17 -O3 -o break-cow-ns4-bitlog src/main.cpp -lpthread -DBREAKDOWN -DMULTIMETA -DNS4 
	g++ -g -std=c++17 -O3 -o break-inp-ns1-minlog src/main.cpp -lpthread -DINPLACE -DBREAKDOWN 
	g++ -g -std=c++17 -O3 -o break-inp-ns2-minlog src/main.cpp -lpthread -DINPLACE -DBREAKDOWN -DMULTIMETA -DNS2 
	g++ -g -std=c++17 -O3 -o break-inp-ns3-minlog src/main.cpp -lpthread -DINPLACE -DBREAKDOWN -DMULTIMETA -DNS3 
	g++ -g -std=c++17 -O3 -o break-inp-ns4-minlog src/main.cpp -lpthread -DINPLACE -DBREAKDOWN -DMULTIMETA -DNS4 
	g++ -g -std=c++17 -O3 -o break-inp-ns1-maxlog src/main.cpp -lpthread -DINPLACE -DBREAKDOWN -DFULLLOG 
	g++ -g -std=c++17 -O3 -o break-inp-ns2-maxlog src/main.cpp -lpthread -DINPLACE -DBREAKDOWN -DMULTIMETA -DNS2 -DFULLLOG 
	g++ -g -std=c++17 -O3 -o break-inp-ns3-maxlog src/main.cpp -lpthread -DINPLACE -DBREAKDOWN -DMULTIMETA -DNS3 -DFULLLOG 
	g++ -g -std=c++17 -O3 -o break-inp-ns4-maxlog src/main.cpp -lpthread -DINPLACE -DBREAKDOWN -DMULTIMETA -DNS4 -DFULLLOG 
bdclean:
	rm break-*

conc:	
	g++ -g -std=c++17 -O3 -o rtree-inp-ns2-minlog src/main.cpp -lpthread -DINPLACE -DMULTIMETA -DNS2
	g++ -g -std=c++17 -O3 -o Share-inp-ns2-minlog src/main.cpp -lpthread -DINPLACE -DSHARED -DMULTIMETA -DNS2
	g++ -g -std=c++17 -O3 -o rtree-cow-ns2-bitlog src/main.cpp -lpthread -DMULTIMETA -DNS2
	g++ -g -std=c++17 -O3 -o Share-cow-ns2-bitlog src/main.cpp -lpthread -DSHARED -DMULTIMETA -DNS2
	g++ -g -std=c++17 -O3 -o concF-inp-ns2-minlog main.cpp   -lpthread -DCONC -INPLACE -DMULTIMETA -DNS2
	g++ -g -std=c++17 -O3 -o concS-inp-ns2-minlog main.cpp   -lpthread -DCONC -INPLACE -DSHARED -DMULTIMETA -DNS2
	g++ -g -std=c++17 -O3 -o concS_2-inp-ns2-minlog main.cpp -lpthread -DCONC -INPLACE -DSHARED_2 -DMULTIMETA -DNS2
	g++ -g -std=c++17 -O3 -o concF-cow-ns2-bitlog main.cpp   -lpthread -DCONC -DMULTIMETA -DNS2
	g++ -g -std=c++17 -O3 -o concS-cow-ns2-bitlog main.cpp   -lpthread -DCONC -DSHARED -DMULTIMETA -DNS2
	g++ -g -std=c++17 -O3 -o concS_2-cow-ns2-bitlog main.cpp -lpthread -DCONC -DSHARED_2 -DMULTIMETA -DNS2
ccclean:
	rm conc?-*
	rm Share-*

test_hdf5:  
	g++ -g -std=c++17 -O3 -o poissonTest main_poisson.cpp $(INCLUDES) $(LDFLAGS) $(LIBS) -lrt -lpthread -DINPLACE                  g++ -g -std=c++17 -O3 -o poissonLockTest main_poisson.cpp $(INCLUDES) $(LDFLAGS) $(LIBS) -lrt -lpthread -DINPLACE -DSHARED  
	g++ -g -std=c++17 -O3 -o hdf5test main.cpp $(INCLUDES) $(LDFLAGS) $(LIBS) -lpthread -DINPLACE -DCONC  
	g++ -g -std=c++17 -O3 -o Lockhdf5test main.cpp $(INCLUDES) $(LDFLAGS) $(LIBS) -lpthread -DINPLACE -DCONC -DSHARED 
	g++ -g -std=c++17 -O3 -o onlySearch main.cpp $(INCLUDES) $(LDFLAGS) $(LIBS) -lpthread -DINPLACE -DCONC -DOS 

