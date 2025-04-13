use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;
use std::collections::HashMap;

#[derive(Clone, Debug)]
struct Component {
    name: String,
    healthy: Arc<Mutex<bool>>,
}

impl Component {
    fn new(name: &str) -> Self {
        Component {
            name: name.to_string(),
            healthy: Arc::new(Mutex::new(true)),
        }
    }

    fn perform_health_check(&self) -> bool {
        // Simulate health check logic
        let is_healthy = rand::random::<f32>() > 0.1; // 90% uptime
        let mut health = self.healthy.lock().unwrap();
        *health = is_healthy;
        println!("Health check for {}: {}", self.name, is_healthy);
        is_healthy
    }

    fn failover(&self) {
        println!("Failover triggered for component: {}", self.name);
    }
}

pub struct RedundancyManager {
    components: HashMap<String, Component>,
}

impl RedundancyManager {
    pub fn new() -> Self {
        RedundancyManager {
            components: HashMap::new(),
        }
    }

    pub fn register_component(&mut self, name: &str) {
        let component = Component::new(name);
        self.components.insert(name.to_string(), component);
    }

    pub fn start_monitoring(&self) {
        let components = self.components.clone();
        thread::spawn(move || loop {
            for (name, comp) in components.iter() {
                if !comp.perform_health_check() {
                    comp.failover();
                }
            }
            thread::sleep(Duration::from_secs(1));
        });
    }
}

fn main() {
    let mut manager = RedundancyManager::new();
    manager.register_component("Router");
    manager.register_component("Executor");
    manager.register_component("Simulator");

    manager.start_monitoring();

    loop {
        thread::sleep(Duration::from_secs(60));
    }
}
