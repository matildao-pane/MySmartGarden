package projectiot;

import java.sql.Timestamp;
import java.util.Date;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapHandler;
import org.eclipse.californium.core.CoapObserveRelation;
import org.eclipse.californium.core.CoapResponse;
import org.json.simple.JSONObject;
import org.json.simple.JSONValue;
import org.json.simple.parser.ParseException;

public class ObservingCoapClient extends CoapClient {
	
	private Resource res;
	CoapObserveRelation coapObserveRelation;
	
	public ObservingCoapClient(Resource res) {
		super(res.getCoapURI());
		this.res= res;
	}
	
	public void startObserving() {
		
		coapObserveRelation = this.observe(new CoapHandler(){
			
			public void onLoad(CoapResponse response) {
				
				try {
					String value="";
					JSONObject jo = (JSONObject) JSONValue.parseWithException(response.getResponseText());
							
					//SET ALL THE STATUS
					if(res.getName().contains("actuator")) {
						value=  jo.get("status").toString();
						if(value.equals("ON")) {
							if(res.getName().contains("irrigator")&&!res.getState()) {	
								System.out.println("\n|| Irrigator started automatically || \n");
							}
							res.setState(true);
						}else {
							if(res.getName().contains("gate")&&res.getState()) {	//if the gate closed automatically in contiki side
								System.out.println("\n|| Gate closed automatically || \n");
							}
							if(res.getName().contains("irrigator")&&res.getState()) {	
								System.out.println("\n|| Irrigator stop automatically || \n");
							}
							
							res.setState(false);
						}
						
						if(res.getName().contains("irrigator")) {
							value= jo.get("auto").toString();
							if(value.equals("1")) {
								res.setAuto(true);
							}else {
								res.setAuto(false);
							}
						}
					}else if(res.getName().contains("sensor")) {
						Long v=(Long) jo.get("value");			
							res.setValue(v);	
					}
					if(Interface.observeMode) {	///if in observe mode
						
						Date date = new Date();
						long time = date.getTime();
						Timestamp timestamp = new Timestamp(time);
						
						System.out.println(res.toString());
						if(jo.containsKey("value")) {
							System.out.println("\t timestamp:" + timestamp);
						}
					}						
				} catch (ParseException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}catch (Exception e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
			
			public void onError() {
				System.err.println("Failed observing----------");
			}
		});
	}
}