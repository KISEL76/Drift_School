CREATE TABLE users (
    user_id SERIAL PRIMARY KEY,
    first_name VARCHAR(20) NOT NULL,
    last_name VARCHAR(20) NOT NULL,
    phone_number VARCHAR(20) UNIQUE NOT NULL,
    email VARCHAR(50) UNIQUE NOT NULL,
    birth_date DATE,
    nickname VARCHAR(15),
    telegram_id BIGINT,
    password_hash VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE instructors (
    instructor_id SERIAL PRIMARY KEY,
    first_name VARCHAR(20) NOT NULL,
    last_name VARCHAR(20) NOT NULL,
    specialization VARCHAR(350),
    phone_number VARCHAR(20) UNIQUE NOT NULL,
    email VARCHAR(50) UNIQUE NOT NULL
);

CREATE TABLE services (
    service_id SERIAL PRIMARY KEY,
    service_name VARCHAR(50) NOT NULL,
    description TEXT,
    price DECIMAL(10, 2) NOT NULL,
    duration_minutes INT NOT NULL
);

CREATE TABLE orders (
    order_id SERIAL PRIMARY KEY,
    user_id INT REFERENCES users(user_id) ON DELETE CASCADE,
    instructor_id INT REFERENCES instructors(instructor_id) ON DELETE SET NULL,
    service_id INT REFERENCES services(service_id) ON DELETE CASCADE,
    order_date TIMESTAMP NOT NULL,
    status VARCHAR(50) DEFAULT 'Ожидает',
    total_price DECIMAL(10, 2) NOT NULL
);

CREATE TABLE reviews (
    review_id SERIAL PRIMARY KEY,
    order_id INT REFERENCES orders(order_id) ON DELETE CASCADE,
    rating INT CHECK (rating BETWEEN 1 AND 5),
    comment TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE schedule (
    schedule_id SERIAL PRIMARY KEY,
    instructor_id INT REFERENCES instructors(instructor_id) ON DELETE CASCADE,
    user_id INT REFERENCES users(user_id) ON DELETE CASCADE,
    available_date DATE NOT NULL,
    start_time TIME NOT NULL,
    end_time TIME NOT NULL,
    is_booked BOOLEAN DEFAULT FALSE
);

CREATE INDEX idx_user_email ON users(email);
CREATE INDEX idx_instructor_email ON instructors(email);
CREATE INDEX idx_service_name ON services(service_name);
CREATE INDEX idx_order_date ON orders(order_date);
CREATE INDEX idx_schedule_date ON schedule(available_date);

INSERT INTO instructors (instructor_id, first_name, last_name, specialization, phone_number, email) VALUES
(1, 'Алексей', 'Петров', 
    'Обучение базовым техникам дрифта для новичков. Автомобиль: Toyota Corolla E90.', 
    '+7 (900) 123-45-67', 
    'alexey.petrov@driftschool.com'),
(2, 'Дмитрий', 'Сидоров', 
    'Профессиональные тренировки дрифта на высокой скорости. Автомобиль: BMW M3 (E46).', 
    '+7 (901) 234-56-78', 
    'dmitry.sidorov@driftschool.com'),
(3, 'Иван', 'Ковалев', 
    'Парный дрифт с акцентом на синхронность и координацию. Автомобиль: Toyota AE86 Trueno.', 
    '+7 (902) 345-67-89', 
    'ivan.kovalev@driftschool.com'),
(4, 'Сергей', 'Новиков', 
    'Продвинутые курсы для опытных пилотов с освоением сложных манёвров. Автомобиль: Nissan Silvia S15.', 
    '+7 (903) 456-78-90', 
    'sergey.novikov@driftschool.com'),
(5, 'Михаил', 'Сергеев', 
    'Тренировки на мокром покрытии для контроля автомобиля при низком сцеплении. Автомобиль: Subaru BRZ.', 
    '+7 (904) 567-89-01', 
    'mikhail.sergeev@driftschool.com');

SELECT setval('instructors_instructor_id_seq', (SELECT MAX(instructor_id) FROM instructors));

INSERT INTO services (
    service_id,
    service_name,
    description,
    price,
    duration_minutes
)
VALUES 
(1, 'Базовый курс дрифта', 'Обучение основам дрифта, включая базовые техники заноса, правильное использование газа и руля. Подходит для начинающих.', 6000.00, 60),
(2, 'Продвинутый курс дрифта', 'Тренировка сложных манёвров, перекладок и контроль автомобиля на высокой скорости. Для опытных пилотов.', 9000.00, 90),
(3, 'Парный дрифт', 'Обучение синхронным манёврам с минимальными дистанциями между автомобилями. Подходит для пилотов, желающих улучшить координацию.', 8000.00, 75),
(4, 'Мокрый дрифт', 'Тренировка на трассе с влажным покрытием. Освоение техник контроля заноса при низком сцеплении.', 7000.00, 60),
(5, 'Дрифт на время', 'Соревновательная тренировка на время, включает оптимизацию траектории и быстрые манёвры.', 8500.00, 90),
(6, 'Корпоративный дрифт', 'Организация тренировок и мероприятий для групп, включая индивидуальные задания и соревнования.', 25000.00, 120),
(7, 'Индивидуальная тренировка', 'Персональный курс с инструктором, разработанный под конкретные запросы клиента.', 12000.00, 60),
(8, 'Детский дрифт', 'Безопасное обучение основам дрифта для детей и подростков. Включает использование автомобилей с ограничением мощности.', 5000.00, 45),
(9, 'Техническая диагностика', 'Оценка состояния автомобиля перед тренировкой, настройка подвески и шин.', 4000.00, 30),
(10, 'Тренировка с симулятором', 'Отработка техник дрифта на профессиональном гоночном симуляторе. Подходит для подготовки в зимний период.', 3000.00, 40);


