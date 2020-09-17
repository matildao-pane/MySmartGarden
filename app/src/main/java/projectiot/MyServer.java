package projectiot;

import org.eclipse.californium.core.CaliforniumLogger;
import org.eclipse.californium.core.CoapServer;

public class MyServer extends CoapServer {
	
	static {
		CaliforniumLogger.disableLogging();
	}
	
	public MyServer(int i) {
		super(i);
		this.add(new RegistrationResource("registration"));
	}

}
