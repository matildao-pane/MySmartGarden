package projectiot;

import java.net.InetAddress;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.server.resources.CoapExchange;

public class RegistrationResource extends CoapResource{

		public RegistrationResource(String name) {
			super(name);
			// TODO Auto-generated constructor stub
		}
		
		public void handleGET(CoapExchange exchange) {  //how to respond to a get request
			//accept request sending ack
			exchange.accept();
		
			//System.out.println("accept");

			//get the source address
			InetAddress addr = exchange.getSourceAddress();
			//System.out.println(addr);    //"/fd00:0:0:0:202:2:2:2"
			
			//create new request for resource discovery
			CoapClient client = new CoapClient("coap://["+ addr.getHostAddress() +"]:5683/.well-known/core");
			CoapResponse response = client.get();
			
			String code = response.getCode().toString();
			//System.out.println(code);
			if(!code.startsWith("2")) {	
				System.err.println("error: " + code);
				return;
			}

			String responseText = response.getResponseText();
			//System.out.println("RESPONSE"+responseText); 
			/*	
			</.well-known/core>;ct=40,
			</light-sensor>;title="Light Sensor";rt="Light Sensor";obs,
			</irrigator-actuator>;title="Watering actuator: ?POST/PUT auto=ON|OFF status=ON|OFF";rt="Irrigator",
			</humidity-sensor>;title="Humidity Sensor";rt="humidity Sensor";obs
			 */	
			
			
			String[] resources = responseText.split(",");
			for(int i = 1; i < resources.length;i++) { //1 the first one is </.well-known/core>;ct=40
				//</hello>   title="Hello world: ?len=0.."    rt="Text" 
				
				try {
					String[] parameters = resources[i].split(";");
					
					String path = parameters[0].split("<")[1].split(">")[0];
					//System.out.println("PATH "+ path);
					String name = path.split("/")[1];
					
					String info = parameters[1]+";"+parameters[2];
					
					boolean obs = false;
					if(parameters.length>3) {
						if(parameters[3].contains("obs")) {
						obs = true;
						}
					}
					
					Resource newRes = new Resource(name, path, addr.getHostAddress(), info, obs);
					
					Interface.registeredResources.put(name,newRes);
					
					if(obs==true) {
						Interface.observedResources.put(name, new ObservingCoapClient(newRes));	
						Interface.observedResources.get(name).startObserving();
					}
					System.out.println("\n"+name+" registered");
					
					
				}catch(Exception e) {
					e.printStackTrace();
				}
			}
		}
}
