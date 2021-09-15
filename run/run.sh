if [ -e "app.wt" ];then
    rm -rf *;
fi
cmake ../app
if [ $? -eq 0 ]; then
    make 
    if [ $? -eq 0 ]; then
        if [ -e WT_PORT ]; then

        else 
            WT_PORT=3000; #default server port is 3000
        fi;
        ./app.wt --docroot ../app/docroot  --approot ../app/approot/ --http-listen 0.0.0.0:$WT_PORT
    fi;
fi;

