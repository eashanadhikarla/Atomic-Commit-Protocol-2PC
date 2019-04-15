for host in ip-172-31-24-210 ip-172-31-22-126 ip-172-31-1-179 ip-172-31-12-241 ip-172-31-13-207 ; do 
   ssh $host ./RHT/threadpool_server
done

for host in ip-172-31-24-210 ip-172-31-22-126 ip-172-31-1-179 ip-172-31-12-241 ip-172-31-13-207 ; do 
   ssh $host ./RHT/client_cordinator
done