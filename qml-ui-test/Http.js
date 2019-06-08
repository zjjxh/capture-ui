function submitPost(m_id, method, par1, par2, par3) {
                var request = new XMLHttpRequest();
                /*
                request.onreadystatechange=function() {
                    // Need to wait for the DONE state or you'll get errors
                    if(request.readyState === XMLHttpRequest.DONE) {
                        if (request.status === 200) {
                            console.log("Response = " + request.responseText);
                            // if response is JSON you can parse it
                            var response = JSON.parse(request.responseText);
                            return response
                            // then do something with it here

                        }
                        else {
                            // This is very handy for finding out why your web service won't talk to you
                            console.log("Status: " + request.status + ", Status Text: " + request.statusText);
                        }
                    }
                }*/
                var url = "http://localhost:5000/motor/" + m_id + "/" + method
                if(method == "move")
                    url += "?angel=" + par1 + "&direction=" + par2 + "&speed=" + par3
                else if(method == "read")
                    url += "?type=" + par1 + "&addr=" + par2 + "&count=" + par3
                request.open("GET",url,false);
                request.send();
                /*
                // Make sure whatever you post is URI encoded
                var encodedString = encodeURIComponent(postString);
                // This is for a POST request but GET etc. work fine too
                request.open("POST", "https://<your_service_endpoint_here>", true); // only async supported
                // You might not need an auth header, or might need to modify - check web service docs
                request.setRequestHeader("Authorization", "Bearer " + yourAccessToken);
                // Post types other than forms should work fine too but I've not tried
                request.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
                // Form data is web service dependent - check parameter format
                var requestString = "text=" + encodedString;
                request.send(requestString);*/
}
