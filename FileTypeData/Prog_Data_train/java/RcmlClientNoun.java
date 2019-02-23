package org.restcomm.connect.rvd.model.steps.dial;


public class RcmlClientNoun extends RcmlNoun {

    public static class Video {
        public Boolean enable;
        public String overlay;
    }

    String destination;
    String url;
    String statusCallback;
    Video video;

    public String getDestination() {
        return destination;
    }
    public void setDestination(String destination) {
        this.destination = destination;
    }

    public String getUrl() {
        return url;
    }

    public void setUrl(String url) {
        this.url = url;
    }

}
