package projectiot;

public class Resource {

	private String info;
	public String name;
	private String path;
	private String address;
	private boolean isObservable;
	private boolean state=false;  //state or value
	private boolean auto=true;
	private Long value = (long) 0;
	
	public Resource (String n,String p, String a, String i, boolean o){
		this.name = n;
		this.path = p;  //  /gate
		this.address = a;
		this.info = i;
		this.isObservable = o;
	}
	public String getName(){ return this.name; }

	public String getInfo(){ return this.info; }
	
	public String getPath(){ return this.path; }
	
	public String getAddress(){ return this.address; }
	
	public boolean isObservable(){ return isObservable; }
	
	public String getCoapURI(){ return "coap://[" + this.address+"]:5683"+ this.path;}
	
	public boolean equals(Resource o){
		return (this.path.equals(o.path) && this.address.equals(o.address));
	}

	public boolean getState() {	return this.state; }
	public void setState(boolean s) {	this.state =s;	}
	public boolean isAuto() {
		return auto;
	}
	public void setAuto(boolean auto) {
		this.auto = auto;
	}
	public Long getValue() {
		return value;
	}
	public void setValue(Long v) {
		this.value = v;
	}

	public String toString() {
		String c = null;
		if(this.getName().contains("actuator")) {
				c = this.getState()?"\t status: ON":"\t status: OFF";		
		}
		if(this.getName().contains("sensor")) {
			c = "\t value: "+this.getValue().toString();
		}
		
		return "\n"+this.getName()+": \n" + c ;
	}
	
}