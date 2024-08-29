#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <SFML/Graphics.hpp>

const int WIDTH = 1200;
const int HEIGHT = 800;
const float GRAVITY = 9.81f;
const float DAMPING = 0.99f;
const float ATTRACTION = 50.0f;
const float REPULSION = 10.0f;
const float MAX_SPEED = 500.0f;

class Particle {
public:
    sf::Vector2f position;
    sf::Vector2f velocity;
    float radius;
    sf::Color baseColor;
    float mass;

    Particle(float x, float y, float r) : position(x, y), velocity(0, 0), radius(r), mass(r * r) {
        baseColor = sf::Color(rand() % 255, rand() % 255, rand() % 255);
    }

    void update(float dt) {
        velocity.y += GRAVITY * dt;
        velocity = limitVelocity(velocity);
        position += velocity * dt;

        if (position.x - radius < 0) {
            position.x = radius;
            velocity.x = -velocity.x * DAMPING;
        } else if (position.x + radius > WIDTH) {
            position.x = WIDTH - radius;
            velocity.x = -velocity.x * DAMPING;
        }

        if (position.y - radius < 0) {
            position.y = radius;
            velocity.y = -velocity.y * DAMPING;
        } else if (position.y + radius > HEIGHT) {
            position.y = HEIGHT - radius;
            velocity.y = -velocity.y * DAMPING;
        }
    }

    void applyForce(const sf::Vector2f& force, float dt) {
        velocity += force * (dt / mass);
    }

    void draw(sf::RenderWindow& window) {
        sf::CircleShape shape(radius);
        shape.setPosition(position - sf::Vector2f(radius, radius));
        
        float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        float hue = (speed / MAX_SPEED) * 360.0f;
        sf::Color color = HSVtoRGB(hue, 1.0f, 1.0f);
        shape.setFillColor(color);
        
        window.draw(shape);
    }

private:
    sf::Vector2f limitVelocity(const sf::Vector2f& vel) {
        float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);
        if (speed > MAX_SPEED) {
            return (vel / speed) * MAX_SPEED;
        }
        return vel;
    }

    sf::Color HSVtoRGB(float H, float S, float V) {
        float C = V * S;
        float X = C * (1 - std::abs(std::fmod(H / 60.0, 2) - 1));
        float m = V - C;
        float Rs, Gs, Bs;

        if(H >= 0 && H < 60) {
            Rs = C; Gs = X; Bs = 0;
        }
        else if(H >= 60 && H < 120) {
            Rs = X; Gs = C; Bs = 0;
        }
        else if(H >= 120 && H < 180) {
            Rs = 0; Gs = C; Bs = X;
        }
        else if(H >= 180 && H < 240) {
            Rs = 0; Gs = X; Bs = C;
        }
        else if(H >= 240 && H < 300) {
            Rs = X; Gs = 0; Bs = C;
        }
        else {
            Rs = C; Gs = 0; Bs = X;
        }

        return sf::Color(
            static_cast<sf::Uint8>((Rs + m) * 255),
            static_cast<sf::Uint8>((Gs + m) * 255),
            static_cast<sf::Uint8>((Bs + m) * 255)
        );
    }
};

class ParticleSystem {
private:
    std::vector<Particle> particles;
    sf::VertexArray connections;

public:
    ParticleSystem() : connections(sf::Lines) {}

    void addParticle(float x, float y, float radius) {
        particles.emplace_back(x, y, radius);
    }

    void update(float dt) {
        for (auto& particle : particles) {
            for (auto& other : particles) {
                if (&particle != &other) {
                    sf::Vector2f direction = other.position - particle.position;
                    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                    
                    if (distance > 0 && distance < 100) {
                        sf::Vector2f force = direction / distance;
                        float strength = ATTRACTION / (distance * distance) - REPULSION / distance;
                        particle.applyForce(force * strength, dt);
                    }
                }
            }
            particle.update(dt);
        }

        updateConnections();
    }

    void updateConnections() {
        connections.clear();
        for (size_t i = 0; i < particles.size(); ++i) {
            for (size_t j = i + 1; j < particles.size(); ++j) {
                float distance = std::sqrt(
                    std::pow(particles[i].position.x - particles[j].position.x, 2) +
                    std::pow(particles[i].position.y - particles[j].position.y, 2)
                );
                if (distance < 100) {
                    sf::Uint8 alpha = static_cast<sf::Uint8>(255 * (1 - distance / 100));
                    connections.append(sf::Vertex(particles[i].position, sf::Color(255, 255, 255, alpha)));
                    connections.append(sf::Vertex(particles[j].position, sf::Color(255, 255, 255, alpha)));
                }
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(connections);
        for (auto& particle : particles) {
            particle.draw(window);
        }
    }

    void addParticlesAtMouse(const sf::Vector2i& mousePos, int count) {
        for (int i = 0; i < count; ++i) {
            float angle = static_cast<float>(rand()) / RAND_MAX * 2 * M_PI;
            float distance = static_cast<float>(rand()) / RAND_MAX * 50;
            float x = mousePos.x + std::cos(angle) * distance;
            float y = mousePos.y + std::sin(angle) * distance;
            addParticle(x, y, 3 + static_cast<float>(rand()) / RAND_MAX * 5);
        }
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Advanced Particle Simulation");
    window.setFramerateLimit(60);

    ParticleSystem particleSystem;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, WIDTH);

    for (int i = 0; i < 100; ++i) {
        particleSystem.addParticle(dis(gen), dis(gen), 3 + dis(gen) / 100);
    }

    sf::Clock clock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    particleSystem.addParticlesAtMouse(sf::Mouse::getPosition(window), 10);
                }
            }
        }

        float dt = clock.restart().asSeconds();

        particleSystem.update(dt);

        window.clear(sf::Color(10, 10, 20));
        particleSystem.draw(window);
        window.display();
    }

    return 0;
}
