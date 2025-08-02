// Main JavaScript for LegacyStream Documentation Website

document.addEventListener('DOMContentLoaded', function() {
    // Theme toggle functionality
    const themeToggle = document.getElementById('themeToggle');
    const body = document.body;
    
    // Check for saved theme preference or default to dark mode
    const savedTheme = localStorage.getItem('theme');
    if (savedTheme) {
        body.className = savedTheme;
    } else {
        body.className = 'dark-mode';
        localStorage.setItem('theme', 'dark-mode');
    }
    
    // Theme toggle event listener
    themeToggle.addEventListener('click', function() {
        if (body.classList.contains('dark-mode')) {
            body.classList.remove('dark-mode');
            localStorage.setItem('theme', '');
        } else {
            body.classList.add('dark-mode');
            localStorage.setItem('theme', 'dark-mode');
        }
    });
    
    // Smooth scrolling for navigation links
    const navLinks = document.querySelectorAll('a[href^="#"]');
    navLinks.forEach(link => {
        link.addEventListener('click', function(e) {
            e.preventDefault();
            const targetId = this.getAttribute('href');
            const targetSection = document.querySelector(targetId);
            
            if (targetSection) {
                const headerHeight = document.querySelector('.header').offsetHeight;
                const targetPosition = targetSection.offsetTop - headerHeight - 20;
                
                window.scrollTo({
                    top: targetPosition,
                    behavior: 'smooth'
                });
            }
        });
    });
    
    // Intersection Observer for animations
    const observerOptions = {
        threshold: 0.1,
        rootMargin: '0px 0px -50px 0px'
    };
    
    const observer = new IntersectionObserver(function(entries) {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.style.opacity = '1';
                entry.target.style.transform = 'translateY(0)';
            }
        });
    }, observerOptions);
    
    // Observe elements for animation
    const animatedElements = document.querySelectorAll('.feature-card, .doc-card, .download-card, .api-example');
    animatedElements.forEach(el => {
        el.style.opacity = '0';
        el.style.transform = 'translateY(20px)';
        el.style.transition = 'opacity 0.6s ease, transform 0.6s ease';
        observer.observe(el);
    });
    
    // Code syntax highlighting (basic)
    const codeBlocks = document.querySelectorAll('.code-block code');
    codeBlocks.forEach(block => {
        const text = block.textContent;
        let highlighted = text;
        
        // Basic syntax highlighting for C++
        highlighted = highlighted
            .replace(/\b(class|void|const|std::|auto|return|if|else|for|while)\b/g, '<span class="keyword">$1</span>')
            .replace(/\b(INVALID_HANDLE_VALUE|NULL|0)\b/g, '<span class="constant">$1</span>')
            .replace(/\b(CreateIoCompletionPort|make_unique|requestLetsEncryptCertificate)\b/g, '<span class="function">$1</span>')
            .replace(/\b(std::string|Codec|Stream)\b/g, '<span class="type">$1</span>')
            .replace(/\b(//.*)/g, '<span class="comment">$1</span>');
        
        block.innerHTML = highlighted;
    });
    
    // Add syntax highlighting CSS
    const style = document.createElement('style');
    style.textContent = `
        .keyword { color: #ff6b6b; font-weight: 500; }
        .constant { color: #4ecdc4; }
        .function { color: #45b7d1; }
        .type { color: #96ceb4; }
        .comment { color: #6c757d; font-style: italic; }
    `;
    document.head.appendChild(style);
    
    // Mobile menu toggle (if needed in future)
    function setupMobileMenu() {
        const mobileMenuButton = document.querySelector('.mobile-menu-toggle');
        const navMenu = document.querySelector('.nav-menu');
        
        if (mobileMenuButton && navMenu) {
            mobileMenuButton.addEventListener('click', function() {
                navMenu.classList.toggle('active');
            });
        }
    }
    
    // Initialize mobile menu
    setupMobileMenu();
    
    // Performance monitoring
    function logPerformance() {
        if ('performance' in window) {
            const perfData = performance.getEntriesByType('navigation')[0];
            console.log('Page load time:', perfData.loadEventEnd - perfData.loadEventStart, 'ms');
        }
    }
    
    // Log performance after page load
    window.addEventListener('load', logPerformance);
    
    // Search functionality (placeholder for future implementation)
    function setupSearch() {
        const searchInput = document.querySelector('.search-input');
        if (searchInput) {
            searchInput.addEventListener('input', function(e) {
                const query = e.target.value.toLowerCase();
                // Implement search logic here
                console.log('Search query:', query);
            });
        }
    }
    
    // Initialize search
    setupSearch();
    
    // Analytics tracking (placeholder)
    function trackEvent(eventName, eventData) {
        // Implement analytics tracking here
        console.log('Event tracked:', eventName, eventData);
    }
    
    // Track page views and interactions
    trackEvent('page_view', {
        page: window.location.pathname,
        title: document.title
    });
    
    // Track button clicks
    document.querySelectorAll('.btn').forEach(btn => {
        btn.addEventListener('click', function() {
            trackEvent('button_click', {
                button_text: this.textContent.trim(),
                button_class: this.className
            });
        });
    });
    
    // Track documentation link clicks
    document.querySelectorAll('.doc-link').forEach(link => {
        link.addEventListener('click', function() {
            trackEvent('documentation_click', {
                doc_title: this.closest('.doc-card').querySelector('.doc-title').textContent,
                doc_url: this.href
            });
        });
    });
    
    // Error handling
    window.addEventListener('error', function(e) {
        console.error('JavaScript error:', e.error);
        trackEvent('javascript_error', {
            message: e.error.message,
            filename: e.filename,
            lineno: e.lineno
        });
    });
    
    // Service Worker registration (for PWA features)
    if ('serviceWorker' in navigator) {
        window.addEventListener('load', function() {
            navigator.serviceWorker.register('/sw.js')
                .then(function(registration) {
                    console.log('ServiceWorker registration successful');
                })
                .catch(function(err) {
                    console.log('ServiceWorker registration failed');
                });
        });
    }
    
    // Keyboard shortcuts
    document.addEventListener('keydown', function(e) {
        // Ctrl/Cmd + K for search
        if ((e.ctrlKey || e.metaKey) && e.key === 'k') {
            e.preventDefault();
            const searchInput = document.querySelector('.search-input');
            if (searchInput) {
                searchInput.focus();
            }
        }
        
        // Ctrl/Cmd + D for theme toggle
        if ((e.ctrlKey || e.metaKey) && e.key === 'd') {
            e.preventDefault();
            themeToggle.click();
        }
    });
    
    // Lazy loading for images (if any are added later)
    function setupLazyLoading() {
        const images = document.querySelectorAll('img[data-src]');
        const imageObserver = new IntersectionObserver((entries, observer) => {
            entries.forEach(entry => {
                if (entry.isIntersecting) {
                    const img = entry.target;
                    img.src = img.dataset.src;
                    img.classList.remove('lazy');
                    imageObserver.unobserve(img);
                }
            });
        });
        
        images.forEach(img => imageObserver.observe(img));
    }
    
    // Initialize lazy loading
    setupLazyLoading();
    
    // Console welcome message
    console.log(`
    🎵 LegacyStream Documentation
    
    Welcome to the LegacyStream documentation website!
    
    Available keyboard shortcuts:
    - Ctrl/Cmd + K: Search
    - Ctrl/Cmd + D: Toggle dark mode
    
    For more information, visit: https://github.com/Legacy-DEV-Team/LegacyStream
    `);
}); 